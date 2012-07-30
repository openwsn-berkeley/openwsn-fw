#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "gpio.h"

    void gpio_init()
   {
          //for dante
      EXTI_InitTypeDef  EXTI_InitStructure;
      NVIC_InitTypeDef  NVIC_InitStructure;
      GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
  
  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2 | GPIO_Pin_1 |GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;// configure as output
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;// configure as output
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_ResetBits(GPIOC, GPIO_Pin_1);// set low RF_SLP_TR_CNTL
//  GPIO_ResetBits(GPIOC, GPIO_Pin_2);
  GPIO_ResetBits(GPIOC, GPIO_Pin_0);
  
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource0);//定义PC1为外部中断
  
  //enable RF interrupt
       // EXTI_InitTypeDef  EXTI_InitStructure; 
      EXTI_InitStructure.EXTI_Line = EXTI_Line0;   //外部中断0
      EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
      EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //上升沿触发
      EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
      EXTI_Init(&EXTI_InitStructure);
      
            //定义外部中断0中断优先级
      NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel; 
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
      NVIC_Init(&NVIC_InitStructure);

   }
