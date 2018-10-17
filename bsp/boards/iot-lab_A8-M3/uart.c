/**
\brief iot-lab_A8-M3 definition of the "uart" bsp module (based on openmoteSTM32 code).

\author Chang Tengfei <tengfei.chang@gmail.com>, July 2012.
\author Alaeddine Weslati <alaeddine.weslati@inria.fr>, January 2014.
\author Elodie Morin <elodie.morin@imag.fr>, July 2015.
*/

#include "stm32f10x_lib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "uart.h"

#include "rcc.h"
#include "nvic.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
    uart_tx_cbt txCb;
    uart_rx_cbt rxCb;
    bool        fXonXoffEscaping;
    uint8_t     xonXoffEscapedByte;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init(void) {
    
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    // reset local variables
    memset(&uart_vars,0,sizeof(uart_vars_t));
    
    // enable GPIO and USART clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    // configure USART TX pin as alternate function push-pull
    GPIO_InitStructure.GPIO_Mode                      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin                       = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed                     = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // configure USART RX as input floating
    GPIO_InitStructure.GPIO_Mode                      = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin                       = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitStructure.USART_BaudRate                = 500000;
    USART_InitStructure.USART_WordLength              = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits                = USART_StopBits_1;
    USART_InitStructure.USART_Parity                  = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl     = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                    = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    
    // make sure no interrupts fire as we enable the UART
    uart_clearTxInterrupts();
    uart_clearRxInterrupts();
    
    // enable USART1
    USART_Cmd(USART1, ENABLE);
    
    // enable NVIC uart
    NVIC_uart();
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
}

void uart_enableInterrupts(void) {
    USART_ITConfig(USART1, USART_IT_TC,   ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void uart_disableInterrupts(void) {
    USART_ITConfig(USART1, USART_IT_TC,   DISABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
}

void uart_clearRxInterrupts(void) {
    USART_ClearFlag(USART1, USART_FLAG_RXNE);
}

void uart_clearTxInterrupts(void) {
    USART_ClearFlag(USART1, USART_FLAG_TC);
}

void    uart_setCTS(bool state) {
    
    if (state==0x01) {
        USART_SendData(USART1,(uint16_t)XON);
    } else {
        USART_SendData(USART1,(uint16_t)XOFF);
    }
}

void uart_writeByte(uint8_t byteToWrite) {
    if (byteToWrite==XON || byteToWrite==XOFF || byteToWrite==XONXOFF_ESCAPE) {
        uart_vars.fXonXoffEscaping     = 0x01;
        uart_vars.xonXoffEscapedByte   = byteToWrite;
        USART_SendData(USART1,(uint16_t)XONXOFF_ESCAPE);
    } else {
        USART_SendData(USART1,(uint16_t)byteToWrite);
    }
}

uint8_t uart_readByte(void) {
    uint16_t temp;

    temp = USART_ReceiveData(USART1);
    return (uint8_t)temp;
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr(void) {
    uart_clearTxInterrupts();
    if (uart_vars.fXonXoffEscaping==0x01) {
        uart_vars.fXonXoffEscaping = 0x00;
        USART_SendData(USART1,(uint16_t)uart_vars.xonXoffEscapedByte^XONXOFF_MASK);
    } else {
        uart_vars.txCb();
    }
    return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr(void) {
    uart_clearRxInterrupts();
    uart_vars.rxCb();
    return DO_NOT_KICK_SCHEDULER;
}
