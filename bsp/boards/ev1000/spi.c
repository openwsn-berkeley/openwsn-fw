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
   uint8_t 		   maxTxBytes;
   spi_return_t    returnType;
   uint8_t*        pNextRxByte;
   uint8_t         maxRxBytes;
   spi_first_t     isFirst;
   spi_last_t      isLast;
   uint8_t		   txrx_bytes_togo;
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

    //enable SPI1 and GPIOA clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
  
	GPIO_InitTypeDef GPIO_InitStructure;
    
    //Configure SPI-related pins: PA.5 as SCLK pin ,PA.7 as MOSI pin, PA.6 as MISO pin, PA.4 as /SEL pin
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);   
    GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);   
    //GPIO_SetBits(GPIOA, GPIO_Pin_4);

    GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
    //Configure SPI1
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; //Full-duplex synchronous transfers on two lines
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;//Master Mode
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b; //8-bit transfer frame format
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;  //the SCK pin has a low-level idle state 
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge; //the first rising edge on the SCK pin is the MSBit capture strobe,
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;//Software NSS mode
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//BaudRate Prescaler = 8
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;//data order with MSB-first
    SPI_InitStructure.SPI_CRCPolynomial     = 7;//CRC Polynomial = 7
    SPI_Init(SPI1, &SPI_InitStructure);
  
    //enable SPI1
    SPI_Cmd(SPI1, ENABLE);
    
#ifdef SPI_IN_INTERRUPT_MODE
    //Configure NVIC: Preemption Priority = 1 and Sub Priority = 1
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                      = SPI1_IRQChannel;
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
	uint8_t snop_byte = 0;
    // register spi frame to send
    spi_vars.pNextTxByte      =  bufTx;
    spi_vars.numTxedBytes     =  0;
    spi_vars.maxTxBytes       =  lenbufTx;
    spi_vars.returnType       =  returnType;
    spi_vars.pNextRxByte      =  bufRx;
    spi_vars.maxRxBytes       =  maxLenBufRx;
    spi_vars.isFirst          =  isFirst;
    spi_vars.isLast           =  isLast;
	spi_vars.txrx_bytes_togo  =  0;
   
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
	// determine how many bytes are to be exchanged in the transaction
	spi_vars.txrx_bytes_togo = spi_vars.maxTxBytes + spi_vars.maxRxBytes;
    // send all bytes
    while (spi_vars.txrx_bytes_togo>0) {
        // write next byte to TX buffer
		if( spi_vars.numTxedBytes < spi_vars.maxTxBytes){
			SPI_I2S_SendData(SPI1,*spi_vars.pNextTxByte);
		} else {
			SPI_I2S_SendData(SPI1,snop_byte);
		}

        // busy wait on the interrupt flag
        while (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);
      
        // clear the interrupt flag
        SPI_I2S_ClearFlag(SPI1,SPI_I2S_FLAG_RXNE);
        // save the byte just received in the RX buffer
        switch (spi_vars.returnType) {
            case SPI_FIRSTBYTE:
                if (spi_vars.numTxedBytes==spi_vars.maxTxBytes) {
                    *spi_vars.pNextRxByte = SPI_I2S_ReceiveData(SPI1);
                }
                break;
            case SPI_BUFFER:
				*spi_vars.pNextRxByte = SPI_I2S_ReceiveData(SPI1);
				if(spi_vars.numTxedBytes >= spi_vars.maxTxBytes){
					spi_vars.pNextRxByte++;
				}
                break;
            case SPI_LASTBYTE:
                *spi_vars.pNextRxByte = SPI_I2S_ReceiveData(SPI1);
                break;
        }
        // one byte less to go
		if( spi_vars.numTxedBytes < spi_vars.maxTxBytes){
			spi_vars.pNextTxByte++;
		}
        spi_vars.numTxedBytes++;
        spi_vars.txrx_bytes_togo--;
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

kick_scheduler_t spi_isr() {
#ifdef SPI_IN_INTERRUPT_MODE
// TODO: fix isr as per spi_txrx code above
    // save the byte just received in the RX buffer
	switch (spi_vars.returnType) {
		case SPI_FIRSTBYTE:
			if (spi_vars.numTxedBytes==0) {
				*spi_vars.pNextRxByte   = SPI_I2S_ReceiveData(SPI1);
			}
			break;
		case SPI_BUFFER:
			if( spi_vars.txrx_bytes_togo <= spi_vars.maxRxBytes){
				*spi_vars.pNextRxByte       = SPI_I2S_ReceiveData(SPI1);
				spi_vars.pNextRxByte++;
			} else {
				SPI_I2S_ReceiveData(SPI1);
			}
			break;
		case SPI_LASTBYTE:
			*spi_vars.pNextRxByte       = SPI_I2S_ReceiveData(SPI1);
			break;
	}
   
    // one byte less to go
    spi_vars.pNextTxByte++;
    spi_vars.numTxedBytes++;
    spi_vars.txrx_bytes_togo--;
   
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
