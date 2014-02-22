/**
\brief openmoteSTM32 definition of the "leds" bsp module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
#include "stm32f10x_lib.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void Delay(void);

//=========================== public ==========================================

void leds_init()
{
    // Enable GPIOC, GPIOB clock 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    // Configure PC.2 and PC.3 as Output push-pull 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // Configure PB.08 and PB.09 as Output push-pull 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// red
void leds_error_on() 
{
  GPIOC->ODR |= 0X0008;
}
void leds_error_off() 
{
   GPIOC->ODR &= ~0X0008;
}
void leds_error_toggle() 
{
   GPIOC->ODR ^= 0X0008;
}
uint8_t leds_error_isOn()
{
  u8 bitstatus = 0x00;
  if ((GPIOC->ODR & 0X0008) != (u32)0)
  {
    bitstatus = 0x01;
  }
  else
  {
    bitstatus = 0x00;
  }
  return bitstatus;
}

void    leds_error_blink()
{
  
}

// green
void    leds_radio_on() 
{
   GPIOC->ODR |= 0X0004;
}
void leds_radio_off() 
{
   GPIOC->ODR &= ~0X0004;
}
void leds_radio_toggle() 
{
   GPIOC->ODR ^= 0X0004;
}
uint8_t leds_radio_isOn() 
{
  u8 bitstatus = 0x00;
  if ((GPIOC->ODR & 0X0004) != (u32)0)
  {
    bitstatus = 0x01;
  }
  else
  {
    bitstatus = 0x00;
  }
  return bitstatus;
}

// blue
void leds_sync_on() 
{
   GPIOB->ODR |= 0X0200;
}
void leds_sync_off() 
{
   GPIOB->ODR &= ~0X0200;
}
void leds_sync_toggle()
{
   GPIOB->ODR ^= 0X0200;
}
uint8_t leds_sync_isOn() 
{
  u8 bitstatus = 0x00;
  if ((GPIOA->ODR & 0X0200) != (u32)0)
  {
    bitstatus = 0x01;
  }
  else
  {
    bitstatus = 0x00;
  }
  return bitstatus;
}
// yellow
void    leds_debug_on()
{
   GPIOB->ODR |= 0X0100;  
}

void    leds_debug_off()
{
   GPIOB->ODR &= ~0X0100;  
}

void    leds_debug_toggle()
{
   GPIOB->ODR ^= 0X0100;  
}

uint8_t leds_debug_isOn()
{
  u8 bitstatus = 0x00;
  if ((GPIOB->ODR & 0X0100) != (u32)0)
  {
    bitstatus = 0x01;
  }
  else
  {
    bitstatus = 0x00;
  }
  return bitstatus;
}

void leds_all_on() 
{
   GPIOC->ODR |= 0X000C;
   GPIOB->ODR |= 0X0300;
}
void leds_all_off() 
{
   GPIOC->ODR &= ~0X000C;
   GPIOB->ODR &= ~0X0300;
}
void leds_all_toggle() 
{
   GPIOC->ODR ^= 0X000C;
   GPIOB->ODR ^= 0X0300;
}

void leds_circular_shift() 
{
     GPIOC->ODR ^= 0X0008;
     Delay();
     GPIOC->ODR ^= 0X0008;
     Delay();
     GPIOC->ODR ^= 0X0004;
     Delay();
     GPIOC->ODR ^= 0X0004;
     Delay();
     GPIOB->ODR ^= 0X0200;
     Delay();
     GPIOB->ODR ^= 0X0200;
     Delay();
     GPIOB->ODR ^= 0X0100;
     Delay();
     GPIOB->ODR ^= 0X0100;
     Delay();
}

void leds_increment()
{
  
}

//=========================== private =========================================

void Delay(void)    //delay
{
  unsigned long ik;
  for(ik=0;ik<0xffff8;ik++) ;
}