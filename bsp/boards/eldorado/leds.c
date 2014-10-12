/**
\brief definition of the "leds" bsp module for Eldo and Jorge boards. 

\author USP and Berkeley colab, February 2012.
*/

#include "leds.h"
#include "eldorado.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {                    
	//initialize led pins as outputs
	LED1DIR = DDIR_OUTPUT;   
	LED2DIR = DDIR_OUTPUT;	
	LED3DIR = DDIR_OUTPUT;
	LED4DIR = DDIR_OUTPUT;
#ifndef ELDORADO_BOARD
	LED5DIR = DDIR_OUTPUT;
#endif
}

void led_error_on() {
   LED1 = LED_ON;
}

void led_error_off() {
	LED1 = LED_OFF;
}
void led_error_toggle() {
	LED1 = ~LED1;
}

void led_radio_on() {
   LED2 = LED_ON;;
}
void led_radio_off() {
	LED2 = LED_OFF;;
}
void led_radio_toggle() {
	LED2 = ~LED2;
}

void led_sync_on() {
	LED3 = LED_ON;
}
void led_sync_off() {
   LED3 = LED_OFF;
}
void led_sync_toggle() {
	LED3 = ~LED3;
}

void led_all_on() {
	LED1 = LED_ON;
	LED2 = LED_ON;
	LED3 = LED_ON;
	LED4 = LED_ON;
}
void led_all_off() {
	LED1 = LED_OFF;
	LED2 = LED_OFF;
	LED3 = LED_OFF;
	LED4 = LED_OFF;
}
void led_all_toggle() {
	LED1 = ~LED1;
	LED2 = ~LED2;
	LED3 = ~LED3;
	LED4 = ~LED4;
}


	bool led_temp;
void leds_circular_shift() {
	if((LED1==LED_OFF)&&(LED2==LED_OFF)&&(LED3==LED_OFF)){              // MAKING A LED ON IF THEY ARE ALL OFF
		LED1=LED_ON;
	}  else if ((LED1==LED_ON)&&(LED2==LED_ON)&&(LED3==LED_ON)){        // MAKING A LED OFF IF THEY ARE ALL ON
		LED1=LED_OFF;
	}
	led_temp= LED3;
	LED3 = LED2;
	LED2 = LED1;    // SWITCHING THE LEDS
	LED1 = led_temp;
}
	unsigned int LED = 0;
void leds_increment() {

	LED = !LED1;
	LED = LED | (!LED2*2);    // AQUIRES THE LEDS STATE
	LED = LED | (!LED3*4);
	LED++;
	if (LED>7){                      // INCREMENT THE LED AMOUNT
		LED=0;}
	LED1 = ~(1&&(LED & 0x01));
	LED2 = ~(1&&(LED & 0x02));           // UPDATE THE LEDS STATE
	LED3 = ~(1&&(LED & 0x04));
}

//=========================== private =========================================