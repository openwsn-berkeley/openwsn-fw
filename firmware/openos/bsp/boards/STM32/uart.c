/**
\brief GINA-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_usart.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "leds.h"

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

    //使能串口1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  
    //******************************************************************************
    //    串口1参数初始化定义部分,串口1参数为38400 ， 8 ，1 ，N  接收中断方式
    //******************************************************************************  
    USART_InitStructure.USART_BaudRate = 38400; //设定传输速率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //设定传输数据位数
    USART_InitStructure.USART_StopBits = USART_StopBits_1;    //设定停止位个数
    USART_InitStructure.USART_Parity = USART_Parity_No ;      //不用校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不用流量控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;   //使用接收和发送功能 
    USART_Init(USART1, &USART_InitStructure);  //初始化串口1
  
    uart_enableInterrupts();
  
    USART_Cmd(USART1, ENABLE);  //使能串口1
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
  
    GPIO_InitTypeDef GPIO_InitStructure;              //定义一个结构体
  
    //******************************************************************************
    //  串口1所使用管脚输出输入定义
    //******************************************************************************
  
    // 定义UART1 TX (PA.09)脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;         //IO口的第九脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //IO口复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);            //初始化串口1输出IO口

    // 定义 USART1 Rx (PA.10)为悬空输入 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;           //IO口的第十脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//IO口悬空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);               //初始化串口1输入IO
  
    NVIC_InitTypeDef 	NVIC_InitStructure;
    //  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//先占优先级2位,从优先级2位
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) 
{
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
}

void uart_enableInterrupts()
{
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ClearFlag(USART1, USART_FLAG_TC);
}

void uart_disableInterrupts()
{
    USART_ITConfig(USART1, USART_IT_TC, DISABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
}

void uart_clearRxInterrupts()
{
    USART_ClearFlag(USART1,USART_FLAG_RXNE);
}

void uart_clearTxInterrupts()
{
    USART_ClearFlag(USART1,USART_FLAG_TC);
}

void uart_writeByte(uint16_t byteToWrite)
{
    USART_SendData(USART1,byteToWrite);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}

uint16_t uart_readByte()
{
    uint16_t temp;
    temp = USART_ReceiveData(USART1);
    return temp;
}

//=========================== interrupt handlers ==============================

uint8_t uart_isr_tx() 
{
    uart_vars.txCb();
    return 0;
}

uint8_t uart_isr_rx() 
{
    uart_vars.rxCb();
    return 0;
}