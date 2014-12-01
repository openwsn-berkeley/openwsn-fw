#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "gpio.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== public ===========================================


void GPIO_Config_ALL_AIN(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // Enable GPIOD and GPIOE clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB 
                         | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD 
                         | RCC_APB2Periph_AFIO, ENABLE);
  
  // PA
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  // PB
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  // PC
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  // PD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}


void GPIO_Configuration(void)
{
}

