/**
\brief openmoteSTM32 definition of the "spi" bsp module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
#include "stm32f10x_lib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "spi.h"
#include "leds.h"

#include "rcc.h"
#include "nvic.h"

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

void spi_init(void) {
   // clear variables
    memset(&spi_vars,0,sizeof(spi_vars_t));
   
    SPI_InitTypeDef  SPI_InitStructure;

    //enable SPI2 and GPIOB, Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
     
    //Configure SPI-related pins: PB.13 as SCLK pin ,PB.14 as MISO pin, PB.15 as MOSI pin, PB.12 as /SEL pin
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIOB->ODR |= 0X1000;//set /SEL high
  
    //Configure SPI2
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; //Full-duplex synchronous transfers on two lines
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;//Master Mode
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b; //8-bit transfer frame format
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;  //the SCK pin has a low-level idle state 
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge; //the first rising edge on the SCK pin is the MSBit capture strobe,
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;//Software NSS mode
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//BaudRate Prescaler = 8 
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;//data order with MSB-first
    SPI_InitStructure.SPI_CRCPolynomial     = 7;//CRC Polynomial = 7
    SPI_Init(SPI2, &SPI_InitStructure);
  
    //enable SPI2 
    SPI_Cmd(SPI2, ENABLE);
    
#ifdef SPI_IN_INTERRUPT_MODE
    //Configure NVIC: Preemption Priority = 1 and Sub Priority = 1
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                      = SPI2_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCallback(spi_cbt cb) {
    spi_vars.callback = cb;
}
#endif

void spi_txrx(uint8_t*     bufTx,
              uint16_t     lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint16_t     maxLenBufRx,
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
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    }
   
#ifdef SPI_IN_INTERRUPT_MODE
    // implementation 1. use a callback function when transaction finishes
   
    // write first byte to TX buffer
    SPI_I2S_SendData(SPI2,*spi_vars.pNextTxByte);
    
    
    // re-enable interrupts
    NVIC_SETPRIMASK();
#else
    // implementation 2. busy wait for each byte to be sent
    // send all bytes
    while (spi_vars.txBytesLeft>0) {
        // write next byte to TX buffer
        SPI_I2S_SendData(SPI2,*spi_vars.pNextTxByte);

        // busy wait on the interrupt flag
        while (SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE) == RESET);
      
        // clear the interrupt flag
        SPI_I2S_ClearFlag(SPI2,SPI_I2S_FLAG_RXNE);
        // save the byte just received in the RX buffer
        switch (spi_vars.returnType) {
            case SPI_FIRSTBYTE:
                if (spi_vars.numTxedBytes==0) {
                    *spi_vars.pNextRxByte   = SPI_I2S_ReceiveData(SPI2);
                }
                break;
            case SPI_BUFFER:
                *spi_vars.pNextRxByte       = SPI_I2S_ReceiveData(SPI2);
                spi_vars.pNextRxByte++;
                break;
            case SPI_LASTBYTE:
                *spi_vars.pNextRxByte       = SPI_I2S_ReceiveData(SPI2);
                break;
        }
        // one byte less to go
        spi_vars.pNextTxByte++;
        spi_vars.numTxedBytes++;
        spi_vars.txBytesLeft--;
    }
   
    // put CS signal high to signal end of transmission to slave
    if (spi_vars.isLast==SPI_LAST) {
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    }
   
    // SPI is not busy anymore
    spi_vars.busy             =  0;
#endif
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t spi_isr(void) {
#ifdef SPI_IN_INTERRUPT_MODE
    // save the byte just received in the RX buffer
    switch (spi_vars.returnType) {
        case SPI_FIRSTBYTE:
            if (spi_vars.numTxedBytes==0) {
                *spi_vars.pNextRxByte   = SPI_I2S_ReceiveData(SPI2);
            }
            break;
        case SPI_BUFFER:
            *spi_vars.pNextRxByte       = SPI_I2S_ReceiveData(SPI2);
            spi_vars.pNextRxByte++;
            break;
        case SPI_LASTBYTE:
            *spi_vars.pNextRxByte       = SPI_I2S_ReceiveData(SPI2);
            break;
    }
   
    // one byte less to go
    spi_vars.pNextTxByte++;
    spi_vars.numTxedBytes++;
    spi_vars.txBytesLeft--;
   
    if (spi_vars.txBytesLeft>0) {
        // write next byte to TX buffer
        SPI_SendData(SPI2,*spi_vars.pNextTxByte);
    } else {
        // put CS signal high to signal end of transmission to slave
        if (spi_vars.isLast==SPI_LAST) {
            GPIO_SetBits(GPIOB, GPIO_Pin_12);
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
