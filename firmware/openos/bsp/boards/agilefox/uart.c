/**
\brief agilefox definition of the "uart" bsp module (based on openmoteSTM32 code).

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
\author Alaeddine Weslati <alaeddine.weslati@inria.fr>,  August 2013.
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

  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 

  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate              = 115200;
  USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits              = USART_StopBits_1;
  USART_InitStructure.USART_Parity                = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);

  USART_Cmd(USART2, ENABLE); // enable USART2
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
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void uart_disableInterrupts()
{
  USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
  USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
}

void uart_clearRxInterrupts()
{
  USART_ClearFlag(USART2, USART_FLAG_RXNE);
}

void uart_clearTxInterrupts()
{
  USART_ClearFlag(USART2, USART_FLAG_TXE);
}

void uart_writeByte(uint8_t byteToWrite)
{ 
  USART_SendData(USART2, byteToWrite);
  while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

  //start or end byte?
  if(byteToWrite == uart_vars.flagByte) {
    uart_vars.startOrend = (uart_vars.startOrend == 0)?1:0;
    //start byte
    if(uart_vars.startOrend == 1) {
      USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    } else {
      USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    }
  }
}

uint8_t uart_readByte()
{
  uint16_t temp;
  temp = USART_ReceiveData(USART2);
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

