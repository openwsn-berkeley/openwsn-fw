/**
\brief GINA-specific definition of the "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stdint.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;  //I/O口的最高输出速度

    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

// red
void leds_error_on() 
{
   GPIO_SetBits(GPIOC, GPIO_Pin_8);
}
void leds_error_off() 
{
   GPIO_ResetBits(GPIOC, GPIO_Pin_8);
}
void leds_error_toggle() 
{
   GPIOC->ODR ^= 0X0100  ;
}
uint8_t leds_error_isOn()
{
    uint8_t temp = GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_8);
    return temp;
}

// green
void    leds_sync_on() 
{
   GPIO_SetBits(GPIOC, GPIO_Pin_6);
}
void leds_sync_off() 
{
   GPIO_ResetBits(GPIOC, GPIO_Pin_6);
}
void leds_sync_toggle() 
{
   GPIOC->ODR ^= 0X0040 ;
}
uint8_t leds_sync_isOn() 
{
   return GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_6);
}

// purple
void leds_radio_on() 
{
   GPIO_SetBits(GPIOC, GPIO_Pin_7);
}
void leds_radio_off() 
{
   GPIO_ResetBits(GPIOC, GPIO_Pin_7);
}
void leds_radio_toggle()
{
   GPIOC->ODR ^= 0X0080;
}
uint8_t leds_radio_isOn() 
{
   return GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_7);
}


void leds_all_on() 
{
   GPIOC->ODR |= 0X01C0;
}
void leds_all_off() 
{
   GPIOC->ODR &= ~0X01C0;
}
void leds_all_toggle() 
{
   GPIOC->ODR ^= 0X01C0;
}

void leds_circular_shift() 
{
  while(1)
  {
     GPIO_SetBits(GPIOC, GPIO_Pin_6);
     Delay();
     GPIO_ResetBits(GPIOC, GPIO_Pin_6);
     Delay();
     GPIO_SetBits(GPIOC, GPIO_Pin_7);
     Delay();
     GPIO_ResetBits(GPIOC, GPIO_Pin_7);
     Delay();
     GPIO_SetBits(GPIOC, GPIO_Pin_8);
     Delay();
     GPIO_ResetBits(GPIOC, GPIO_Pin_8);
     Delay();
  }
}

//=========================== private =========================================

void Delay(void)    //延时函数
{
  unsigned long ik;
  for(ik=0;ik<0xffff8;ik++) ;
}