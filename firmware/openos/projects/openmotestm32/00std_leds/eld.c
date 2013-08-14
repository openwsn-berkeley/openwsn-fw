
#include "stdio.h"
#include "stm32f10x_lib.h"
int main()
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |//72M
	                         RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD ,ENABLE);

  //JTAG-DPÊ§ÄÜ + SW-DPÊ¹ÄÜ
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_6 ; 
  GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;  
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  while(1)
  {
    GPIOC->ODR ^= 1<<6;
    int i, j;
    for(i = 0; i < 50000; i++)
    {
       for(j=0;j<19;j++) ;	  // 4 Clock per time ,total 72 times for 1us
    }	
  }
}

