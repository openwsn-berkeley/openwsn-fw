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
    // Enable GPIOC clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

// red
void leds_error_on() 
{
  GPIOC->ODR |= 0X0040;
}
void leds_error_off() 
{
   GPIOC->ODR &= ~0X0040;
}
void leds_error_toggle() 
{
   GPIOC->ODR ^= 0X0040;
}
uint8_t leds_error_isOn()
{
  u8 bitstatus = 0x00;
  if ((GPIOC->ODR & 0X0040) != (u32)0)
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
  for(int i=0;i<16;i++) {
    leds_error_toggle();
    Delay();
  }
  
}

// green
void    leds_radio_on() 
{
   GPIOC->ODR |= 0X0080;
}
void leds_radio_off() 
{
   GPIOC->ODR &= ~0X0080;
}
void leds_radio_toggle() 
{
   GPIOC->ODR ^= 0X0080;
}
uint8_t leds_radio_isOn() 
{
  u8 bitstatus = 0x00;
  if ((GPIOC->ODR & 0X0080) != (u32)0)
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
   GPIOC->ODR |= 0X0400;
}
void leds_sync_off() 
{
   GPIOC->ODR &= ~0X0400;
}
void leds_sync_toggle()
{
   GPIOC->ODR ^= 0X0400;
}
uint8_t leds_sync_isOn() 
{
  u8 bitstatus = 0x00;
  if ((GPIOC->ODR & 0X0400) != (u32)0)
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
   GPIOC->ODR |= 0X0800;  
}

void    leds_debug_off()
{
   GPIOC->ODR &= ~0X0800;  
}

void    leds_debug_toggle()
{
   GPIOC->ODR ^= 0X0800;  
}

uint8_t leds_debug_isOn()
{
  u8 bitstatus = 0x00;
  if ((GPIOC->ODR & 0X0800) != (u32)0)
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
   GPIOC->ODR |= 0X0CC0;
}
void leds_all_off() 
{
   GPIOC->ODR &= ~0X0CC0;
}
void leds_all_toggle() 
{
   GPIOC->ODR ^= 0X0CC0;
}

void leds_circular_shift() 
{
     GPIOC->ODR ^= 0X0040;
     Delay();
     GPIOC->ODR ^= 0X0040;
     Delay();
     GPIOC->ODR ^= 0X0080;
     Delay();
     GPIOC->ODR ^= 0X0080;
     Delay();
     GPIOC->ODR ^= 0X0400;
     Delay();
     GPIOC->ODR ^= 0X0400;
     Delay();
     GPIOC->ODR ^= 0X0800;
     Delay();
     GPIOC->ODR ^= 0X0800;
     Delay();
}

void leds_increment()
{
  
}

//=========================== private =========================================

void Delay(void)    //delay
{
  unsigned long ik;
  for(ik=0;ik<0x7fff8;ik++) ;
}