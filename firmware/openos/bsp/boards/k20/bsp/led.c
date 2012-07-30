
/**
\brief Low level led macros.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
 */


#include "led.h"


void GPIO_Init(void)
{
  ENABLE_GPIO_CLOCKS;
  
  LED0_EN;
  LED1_EN;
  LED2_EN;
  LED3_EN;

 // LEDs_On();
  LED_Dir_Out();
}

void GPIO_DeInit(void)
{
  
	  LED0_OFF;
	  LED1_OFF;
	  LED2_OFF;
	  LED3_OFF;
	  
  DISABLE_GPIO_CLOCKS;
  

}

void LED_Dir_Out(void)
{
#ifdef CPU_MK60N512VMD100
  GPIOA_POER = (1<<10)|(1<<11)|(1<<28)|(1<<29);
#elif (defined(CPU_MK40N512VMD100))
  GPIOB_POER = (1<<11);
  GPIOC_POER = ((1<<7)|(1<<8)|(1<<9));
#elif (defined(MCU_MK20D7)||defined(MCU_MK20DZ10))
#ifdef TOWER_K20
  GPIOC_PDDR |= ((1<<7)|(1<<8)|(1<<9)|(1<<10));
#elif OPENMOTE_K20
  GPIOB_PDDR |= ((1<<2)|(1<<3)|(1<<10)|(1<<11));
#endif
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
#ifdef TOWER_K20
  GPIOC_PDOR &= ~((1<<9)|(1<<10));
  GPIOC_PDOR |= ((1<<7)|(1<<8));
#elif OPENMOTE_K20
  GPIOC_PDOR &= ~((1<<2)|(1<<3)|(1<<10)|(1<<11));
#endif 
#endif
}





