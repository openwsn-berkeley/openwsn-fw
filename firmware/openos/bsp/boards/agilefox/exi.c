
// Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include "stm32f10x_gpio.h"

//*******************************************************************************
// 函数名称    : void Exti_Init(void)
// 功能描述    : 外部中断初始化定义函数.
// 输入        : None
// 输出        : None
// 返回        : None
//******************************************************************************/
void Exti_Init(void)
   {
      EXTI_InitTypeDef  EXTI_InitStructure;
      NVIC_InitTypeDef  NVIC_InitStructure;

      GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0);//定义PE0为外部中断
      GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource1);//定义PE1为外部中断
       
      EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
      EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
      EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发

      EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
      EXTI_Init(&EXTI_InitStructure);

     // EXTI_InitTypeDef  EXTI_InitStructure; 
      EXTI_InitStructure.EXTI_Line = EXTI_Line1;   //外部中断1
      EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
      EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //上升沿触发
      EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
      EXTI_Init(&EXTI_InitStructure);
       
      //定义外部中断0中断优先级
      NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel; 
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =3;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
      NVIC_Init(&NVIC_InitStructure);

       //定义外部中断1中断优先级
      NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
      NVIC_Init(&NVIC_InitStructure);
}


