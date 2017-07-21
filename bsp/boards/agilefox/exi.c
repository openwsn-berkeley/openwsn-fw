
// Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include "stm32f10x_gpio.h"

//*******************************************************************************
// ��������    : void Exti_Init(void)
// ��������    : �ⲿ�жϳ�ʼ�����庯��.
// ����        : None
// ���        : None
// ����        : None
//******************************************************************************/
void Exti_Init(void)
   {
      EXTI_InitTypeDef  EXTI_InitStructure;
      NVIC_InitTypeDef  NVIC_InitStructure;

      GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0);//����PE0Ϊ�ⲿ�ж�
      GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource1);//����PE1Ϊ�ⲿ�ж�
       
      EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
      EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
      EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���

      EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
      EXTI_Init(&EXTI_InitStructure);

     // EXTI_InitTypeDef  EXTI_InitStructure; 
      EXTI_InitStructure.EXTI_Line = EXTI_Line1;   //�ⲿ�ж�1
      EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
      EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //�����ش���
      EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
      EXTI_Init(&EXTI_InitStructure);
       
      //�����ⲿ�ж�0�ж����ȼ�
      NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel; 
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =3;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
      NVIC_Init(&NVIC_InitStructure);

       //�����ⲿ�ж�1�ж����ȼ�
      NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
      NVIC_Init(&NVIC_InitStructure);
}


