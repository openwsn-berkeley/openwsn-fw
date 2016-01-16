/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: EZR32WG-specific definition of the "leds" bsp module.
 */

#include "stdint.h"
#include "leds.h"
#include "board.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
}

// red
void leds_error_on() {
}
void leds_error_off() {
}
void leds_error_toggle() {
}
uint8_t leds_error_isOn() {
	return 0;
}

// orange
void leds_sync_on() {
}
void leds_sync_off() {
}
void leds_sync_toggle() {
}
uint8_t leds_sync_isOn() {
	return 0;
}

// green
void leds_radio_on() {
}
void leds_radio_off() {
}
void leds_radio_toggle() {
}
uint8_t leds_radio_isOn() {
	return 0;
}

// yellow
void leds_debug_on() {
}
void leds_debug_off() {
}
void leds_debug_toggle() {
}
uint8_t leds_debug_isOn() {
	return 0;
}

// all
void leds_all_on() {
}
void leds_all_off() {

}
void leds_all_toggle() {

}

void leds_error_blink() {
	uint8_t i;
	volatile uint16_t delay;

	// turn all LEDs off
	//bspLedClear(BSP_LED_ALL);

	// blink error LED for ~10s
	for (i = 0; i < 80; i++) {
		//bspLedToggle(BSP_LED_1);
		for (delay = 0xffff; delay > 0; delay--)
			;
		for (delay = 0xffff; delay > 0; delay--)
			;
	}
}

void leds_circular_shift() {
}

void leds_increment() {
}

//=========================== private =========================================


