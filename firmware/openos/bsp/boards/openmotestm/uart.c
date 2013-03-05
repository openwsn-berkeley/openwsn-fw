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
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init() 
{
    // reset local variables
    memset(&uart_vars,0,sizeof(uart_vars_t));
  
    USART_InitTypeDef USART_InitStructure;

    // Enable UART4 and GPIOA clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
    
    //configure UART4 :Baud rate = 38400bit/s, data bit = 8bit, stop bit = 1, no parity, no flow control enable Tx and Rx 
    USART_InitStructure.USART_BaudRate            = 38400; 
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b; 
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;    
    USART_InitStructure.USART_Parity              = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USART_InitStructure);

    //uart_disableInterrupts(); 
    //enable usart
    USART_Cmd(UART4, ENABLE);
  
    // Configure PA.09 as alternate function push-pull
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    //Configure PA.10 as input floating 
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
  
//    //Configure NVIC: Preemption Priority = 3 and Sub Priority = 3
//    NVIC_InitTypeDef 	NVIC_InitStructure;
//    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQChannel;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) 
{
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
}

void uart_enableInterrupts()
{
    USART_ITConfig(UART4, USART_IT_TC, ENABLE);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_ClearFlag(UART4, USART_FLAG_TC);
}

void uart_disableInterrupts()
{
    USART_ITConfig(UART4, USART_IT_TC, DISABLE);
    USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
}

void uart_clearRxInterrupts()
{
    USART_ClearFlag(UART4,USART_FLAG_RXNE);
}

void uart_clearTxInterrupts()
{
    USART_ClearFlag(UART4,USART_FLAG_TC);
}

void uart_writeByte(uint8_t byteToWrite)
{
    USART_SendData(UART4,(uint16_t)byteToWrite);
    while(USART_GetFlagStatus(UART4,USART_FLAG_TXE) == RESET);
}

uint8_t uart_readByte()
{
    uint16_t temp;
    temp = USART_ReceiveData(UART4);
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