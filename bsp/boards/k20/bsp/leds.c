/**
\brief K20-specific definition of the "leds" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, April 2012.
 */

#include "leds.h"
#include "led.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
	GPIO_Init();
}

void leds_deinit() {
	GPIO_DeInit();
}

// red led3
void    leds_error_on() {
	LED1_ON;  
}
void    leds_error_off() {
	LED1_OFF;
}
void    leds_error_toggle() {
	LED1_TOGGLE;
}
uint8_t leds_error_isOn() {
	return LED1_IS_ON;
}

void    leds_error_blink(){
	   uint8_t i;
	   volatile uint16_t delay;
	   // turn all LEDs off
	   leds_all_off();
	     
	   // blink error LED for ~10s
	   for (i=0;i<80;i++) {
		  LED1_TOGGLE; //10 seconds more or less..
	      for (delay=0xffff;delay>0;delay--);
	      for (delay=0xffff;delay>0;delay--);
	   }
}


// orange led 1
void    leds_radio_on() {
	LED3_ON;
}
void    leds_radio_off() {
	LED3_OFF;
}
void    leds_radio_toggle() {
	LED3_TOGGLE;
}
uint8_t leds_radio_isOn() {
 return LED3_IS_ON;
}

// green led2
void    leds_sync_on() {
	LED2_ON;
}
void    leds_sync_off() {
	LED2_OFF;
}
void    leds_sync_toggle() {
	LED2_TOGGLE;
}
uint8_t leds_sync_isOn() {
	return LED2_IS_ON;
}

// yellow led 4
void    leds_debug_on() {
	LED0_ON;
}
void    leds_debug_off() {
	LED0_OFF;
}
void leds_debug_toggle() {
	LED0_TOGGLE;
}
uint8_t leds_debug_isOn() {
	return LED0_IS_ON;
}

void leds_all_on() {
	LED0_ON;
	LED1_ON;
	LED2_ON;
	LED3_ON;
}
void leds_all_off() {
	LED0_OFF;
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;
}
void leds_all_toggle() {
	LED0_TOGGLE;
	LED1_TOGGLE;
	LED2_TOGGLE;
	LED3_TOGGLE;
}

void leds_circular_shift() {
	leds_all_toggle();
}

void leds_increment() {
	leds_all_toggle();
}

//=========================== private =========================================