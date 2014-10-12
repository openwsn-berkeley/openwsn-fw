/**
\brief Low level led macros.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
 */

#ifndef __LED_H
#define __LED_H


#include "common.h"

/* Function prototypes */
void GPIO_Init(void);
void LEDs_On(void);
void LED_Dir_Out(void);
void GPIO_DeInit(void);


#ifdef CPU_MK60N512VMD100

  #define ENABLE_GPIO_CLOCKS (SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK))
  #define LED0_EN (PORTA_PCR11 = PORT_PCR_MUX(1))
  #define LED1_EN (PORTA_PCR29 = PORT_PCR_MUX(1))
  #define LED2_EN (PORTA_PCR28 = PORT_PCR_MUX(1))
  #define LED3_EN (PORTA_PCR10 = PORT_PCR_MUX(1))

  #define LED0_TOGGLE (GPIOA_PTOR = (1<<11))
  #define LED1_TOGGLE (GPIOA_PTOR = (1<<29))
  #define LED2_TOGGLE (GPIOA_PTOR = (1<<28))
  #define LED3_TOGGLE (GPIOA_PTOR = (1<<10))

#elif (defined(CPU_MK40N512VMD100))

  #define ENABLE_GPIO_CLOCKS (SIM_SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK))
  #define LED0_EN (PORTB_PCR11 = PORT_PCR_MUX(1))
  #define LED1_EN (PORTC_PCR7 = PORT_PCR_MUX(1))
  #define LED2_EN (PORTC_PCR8 = PORT_PCR_MUX(1))
  #define LED3_EN (PORTC_PCR9 = PORT_PCR_MUX(1))

  #define LED0_TOGGLE (GPIOB_PTOR = (1<<11))
  #define LED1_TOGGLE (GPIOC_PTOR = (1<<7))
  #define LED2_TOGGLE (GPIOC_PTOR = (1<<8))
  #define LED3_TOGGLE (GPIOC_PTOR = (1<<9))

#elif (defined(MCU_MK20D7)||defined(MCU_MK20DZ10))
 
	#ifdef TOWER_K20
	#define ENABLE_GPIO_CLOCKS (SIM_SCGC5 |= (SIM_SCGC5_PORTC_MASK))
	 #define DISABLE_GPIO_CLOCKS (SIM_SCGC5 &= ~(SIM_SCGC5_PORTC_MASK))
	 /* TWR-K20D72M GPIO for LED configuration 
	 *		Diode D7 - PTC7
	 *		Diode D8 - PTC8
	 *		Diode D9 - PTC9
	 * 	Diode D10 - PTC10
	 */
	  #define LED0_EN (PORTC_PCR7 = PORT_PCR_MUX(1))	//set as gpio
	  #define LED1_EN (PORTC_PCR8 = PORT_PCR_MUX(1))
	  #define LED2_EN (PORTC_PCR9 = PORT_PCR_MUX(1))
	  #define LED3_EN (PORTC_PCR10 = PORT_PCR_MUX(1))
	  
	  #define LED0_TOGGLE (GPIOC_PTOR |= (1<<7))
	  #define LED1_TOGGLE (GPIOC_PTOR |= (1<<8))
	  #define LED2_TOGGLE (GPIOC_PTOR |= (1<<9))
	  #define LED3_TOGGLE (GPIOC_PTOR |= (1<<10))
	
	  #define LED0_ON (GPIOC_PSOR |= (1<<7))
	  #define LED1_ON (GPIOC_PSOR |= (1<<8))
	  #define LED2_ON (GPIOC_PSOR |= (1<<9))
	  #define LED3_ON (GPIOC_PSOR |= (1<<10))
	
	  #define LED0_OFF (GPIOC_PCOR |= (1<<7))
	  #define LED1_OFF (GPIOC_PCOR |= (1<<8))
	  #define LED2_OFF (GPIOC_PCOR |= (1<<9))
	  #define LED3_OFF (GPIOC_PCOR |= (1<<10))
	
	  #define LED0_IS_ON ((GPIOC_PSOR & (1<<7))>>7)
	  #define LED1_IS_ON ((GPIOC_PSOR & (1<<8))>>8)
	  #define LED2_IS_ON ((GPIOC_PSOR & (1<<9))>>9)
	  #define LED3_IS_ON ((GPIOC_PSOR & (1<<10))>>10)
	
	#elif OPENMOTE_K20
	 #define ENABLE_GPIO_CLOCKS (SIM_SCGC5 |= (SIM_SCGC5_PORTB_MASK))
	 #define DISABLE_GPIO_CLOCKS (SIM_SCGC5 &= ~(SIM_SCGC5_PORTB_MASK))
	
	 #define LED0_EN (PORTB_PCR2 = PORT_PCR_MUX(1))	//set as gpio
	 #define LED1_EN (PORTB_PCR3 = PORT_PCR_MUX(1))
	 #define LED2_EN (PORTB_PCR10 = PORT_PCR_MUX(1))
	 #define LED3_EN (PORTB_PCR11 = PORT_PCR_MUX(1))
	 
	 #define LED0_TOGGLE (GPIOB_PTOR |= (1<<2))
	 #define LED1_TOGGLE (GPIOB_PTOR |= (1<<3))
	 #define LED2_TOGGLE (GPIOB_PTOR |= (1<<10))
	 #define LED3_TOGGLE (GPIOB_PTOR |= (1<<11))
	
	 #define LED0_ON (GPIOB_PCOR |= (1<<2))
	 #define LED1_ON (GPIOB_PCOR |= (1<<3))
	 #define LED2_ON (GPIOB_PCOR |= (1<<10))
	 #define LED3_ON (GPIOB_PCOR |= (1<<11))
	
	 #define LED0_OFF (GPIOB_PSOR |= (1<<2))
	 #define LED1_OFF (GPIOB_PSOR |= (1<<3))
	 #define LED2_OFF (GPIOB_PSOR |= (1<<10))
	 #define LED3_OFF (GPIOB_PSOR |= (1<<11))
	
	 #define LED0_IS_ON ((GPIOB_PCOR & (1<<2))>>2)
	 #define LED1_IS_ON ((GPIOB_PCOR & (1<<3))>>3)
	 #define LED2_IS_ON ((GPIOB_PCOR & (1<<10))>>10)
	 #define LED3_IS_ON ((GPIOB_PCOR & (1<<11))>>11)
	 
	#endif
#endif

#endif
