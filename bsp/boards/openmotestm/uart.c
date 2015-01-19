/**
\brief openmoteSTM32 definition of the "uart" bsp module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

#include "stm32f10x_lib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "uart.h"
#include "leds.h"

#include "rcc.h"
#include "nvic.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
   uint8_t     startOrend;
   uint8_t     flagByte;
   bool        flag;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init() 
{
    // reset local variables
    memset(&uart_vars,0,sizeof(uart_vars_t));
    
    //when this value is 0, we are send the first data
    uart_vars.startOrend = 0;
    //flag byte for start byte and end byte
    uart_vars.flagByte = 0x7E;
  
    USART_InitTypeDef USART_InitStructure;

    // Enable USART1 and GPIOC clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    
    //configure USART1 :Baud rate = 115200bit/s, data bit = 8bit, stop bit = 1, no parity, no flow control enable Tx and Rx 
    USART_InitStructure.USART_BaudRate            = 115200; 
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b; 
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;    
    USART_InitStructure.USART_Parity              = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    //enable usart
    USART_Cmd(USART1, ENABLE);
  
    // Configure PA.9 as alternate function push-pull
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //Configure PA.10 as input floating 
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) 
{
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
    
    //enable nvic uart.
     NVIC_uart();
}

void uart_enableInterrupts()
{
//    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void uart_disableInterrupts()
{
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
}

void uart_clearRxInterrupts()
{
    USART_ClearFlag(USART1,USART_FLAG_RXNE);
}

void uart_clearTxInterrupts()
{
    USART_ClearFlag(USART1,USART_FLAG_TXE);
}

void uart_writeByte(uint8_t byteToWrite)
{ 
    USART_SendData(USART1,(uint16_t)byteToWrite);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
      //start or end byte?
    if(byteToWrite == uart_vars.flagByte){
      uart_vars.startOrend = (uart_vars.startOrend == 0)?1:0;
      //start byte
      if(uart_vars.startOrend == 1)
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
      else
        USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    }
}

uint8_t uart_readByte()
{
    uint16_t temp;
    temp = USART_ReceiveData(USART1);
    return (uint8_t)temp;
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr() 
{
    uart_vars.txCb();
    return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr() 
{
    uart_vars.rxCb();
    return DO_NOT_KICK_SCHEDULER;
}