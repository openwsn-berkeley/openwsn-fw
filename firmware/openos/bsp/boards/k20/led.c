#include "led.h"

/********************************************************************************
 *   GPIO_Init: Initializes GPIO controlling LED
 * Notes:
 *    - 
 ********************************************************************************/
void GPIO_Init(void)
{
  ENABLE_GPIO_CLOCKS;
  
  LED0_EN;
  LED1_EN;
  LED2_EN;
  LED3_EN;

  LEDs_On();
  LED_Dir_Out();
}

void LED_Dir_Out(void)
{
#ifdef CPU_MK60N512VMD100
  GPIOA_POER = (1<<10)|(1<<11)|(1<<28)|(1<<29);
#elif (defined(CPU_MK40N512VMD100))
  GPIOB_POER = (1<<11);
  GPIOC_POER = ((1<<7)|(1<<8)|(1<<9));
#elif (defined(MCU_MK20D7))
  GPIOC_PDDR = ((1<<7)|(1<<8)|(1<<9)|(1<<10));
#endif
}

void LEDs_On(void)
{
#ifdef CPU_MK60N512VMD100
  GPIOA_PDOR &= ~((1<<10)|(1<<11)|(1<<28)|(1<<29));
#elif (defined(CPU_MK40N512VMD100))
  GPIOB_PDOR &= ~(1<<11);
  GPIOC_PDOR &= ~((1<<7)|(1<<8)|(1<<9));
#elif (defined(MCU_MK20D7))
  GPIOC_PDOR &= ~((1<<9)|(1<<10));	
  GPIOC_PDOR |= (1<<7)|(1<<8);	
#endif
}


