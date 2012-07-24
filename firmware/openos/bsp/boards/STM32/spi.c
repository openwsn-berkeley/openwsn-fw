/**
\brief GINA-specific definition of the "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/
#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "spi.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   // information about the current transaction
   uint8_t*        pNextTxByte;
   uint8_t         numTxedBytes;
   uint8_t         txBytesLeft;
   spi_return_t    returnType;
   uint8_t*        pNextRxByte;
   uint8_t         maxRxBytes;
   spi_first_t     isFirst;
   spi_last_t      isLast;
   // state of the module
   uint8_t         busy;
#ifdef SPI_IN_INTERRUPT_MODE
   // callback when module done
   spi_cbt         callback;
#endif
} spi_vars_t;

spi_vars_t spi_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void spi_init() {
   // clear variables
   memset(&spi_vars,0,sizeof(spi_vars_t));
   
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  //使能SPI_1时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
     
  //定义 SPI1 : SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  //定义片选线IO口PA4
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  // SPI1 定义
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI设置为双线双向全双工 
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //设置为主 SPI 
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //SPI发送接收 8 位帧结构 
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;  //时钟悬空低 
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //数据捕获于第一个时钟沿 
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;//软件控制 NSS 信号 
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//波特率预分频值为8 
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//数据传输从 MSB 位开始 
  SPI_InitStructure.SPI_CRCPolynomial = 7;//定义了用于 CRC值计算的多项式 7
  SPI_Init(SPI1, &SPI_InitStructure);
  
//  SPI_I2S_ITConfig(SPI1,SPI_I2S_IT_RXNE,ENABLE);
  
  //使能 SPI1 
  SPI_Cmd(SPI1, ENABLE);
#ifdef SPI_IN_INTERRUPT_MODE
  NVIC_InitTypeDef 	NVIC_InitStructure;
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//先占优先级2位,从优先级2位
  NVIC_InitStructure.NVIC_IRQChannel	=	SPI1_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	=	1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority	=	1;
  NVIC_InitStructure.NVIC_IRQChannelCmd	=	ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCallback(spi_cbt cb) {
   spi_vars.callback = cb;
}
#endif

void spi_txrx(uint8_t*     bufTx,
              uint8_t      lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint8_t      maxLenBufRx,
              spi_first_t  isFirst,
              spi_last_t   isLast) {

#ifdef SPI_IN_INTERRUPT_MODE
   // disable interrupts
   NVIC_RESETPRIMASK();
#endif
   
   // register spi frame to send
   spi_vars.pNextTxByte      =  bufTx;
   spi_vars.numTxedBytes     =  0;
   spi_vars.txBytesLeft      =  lenbufTx;
   spi_vars.returnType       =  returnType;
   spi_vars.pNextRxByte      =  bufRx;
   spi_vars.maxRxBytes       =  maxLenBufRx;
   spi_vars.isFirst          =  isFirst;
   spi_vars.isLast           =  isLast;
   
   // SPI is now busy
   spi_vars.busy             =  1;
   
   
   // lower CS signal to have slave listening
   if (spi_vars.isFirst==SPI_FIRST) {
   GPIO_ResetBits(GPIOA, GPIO_Pin_4);
   }
   
#ifdef SPI_IN_INTERRUPT_MODE
   // implementation 1. use a callback function when transaction finishes
   
   // write first byte to TX buffer
   SPI_I2S_SendData(SPI1,*spi_vars.pNextTxByte);
   
   
   // re-enable interrupts
   NVIC_SETPRIMASK();
#else
   // implementation 2. busy wait for each byte to be sent
   // send all bytes
   while (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
   SPI_I2S_SendData(SPI1,*spi_vars.pNextTxByte);

      // busy wait on the interrupt flag
      while (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);
      
      // clear the interrupt flag
      SPI_I2S_ClearFlag(SPI1,SPI_I2S_FLAG_RXNE);
      // save the byte just received in the RX buffer
      switch (spi_vars.returnType) {
         case SPI_FIRSTBYTE:
            if (spi_vars.numTxedBytes==0) {
               *spi_vars.pNextRxByte   = SPI_I2S_ReceiveData(SPI1);
            }
            break;
         case SPI_BUFFER:
            *spi_vars.pNextRxByte      = SPI_I2S_ReceiveData(SPI1);
            spi_vars.pNextRxByte++;
            break;
         case SPI_LASTBYTE:
            *spi_vars.pNextRxByte      = SPI_I2S_ReceiveData(SPI1);
            break;
      }
      // one byte less to go
      spi_vars.pNextTxByte++;
      spi_vars.numTxedBytes++;
      spi_vars.txBytesLeft--;
   }
   
   // put CS signal high to signal end of transmission to slave
   if (spi_vars.isLast==SPI_LAST) {
   GPIO_SetBits(GPIOA, GPIO_Pin_4);
   }
   
   // SPI is not busy anymore
   spi_vars.busy             =  0;
#endif
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

uint8_t spi_isr() {
#ifdef SPI_IN_INTERRUPT_MODE
   // save the byte just received in the RX buffer
   switch (spi_vars.returnType) {
      case SPI_FIRSTBYTE:
         if (spi_vars.numTxedBytes==0) {
            *spi_vars.pNextRxByte = SPI_I2S_ReceiveData(SPI1);
         }
         break;
      case SPI_BUFFER:
         *spi_vars.pNextRxByte    = SPI_I2S_ReceiveData(SPI1);
         spi_vars.pNextRxByte++;
         break;
      case SPI_LASTBYTE:
         *spi_vars.pNextRxByte    = SPI_I2S_ReceiveData(SPI1);
         break;
   }
   
   // one byte less to go
   spi_vars.pNextTxByte++;
   spi_vars.numTxedBytes++;
   spi_vars.txBytesLeft--;
   
   if (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
   SPI_SendData(SPI1,*spi_vars.pNextTxByte);
   } else {
      // put CS signal high to signal end of transmission to slave
      if (spi_vars.isLast==SPI_LAST) {
   GPIO_SetBits(GPIOA, GPIO_Pin_4);
      }
      // SPI is not busy anymore
      spi_vars.busy          =  0;
      
      // SPI is done!
      if (spi_vars.callback!=NULL) {
         // call the callback
         spi_vars.callback();
         // kick the OS
         return 1;
      }
   }
#else
   while(1);// this should never happen
#endif
}
