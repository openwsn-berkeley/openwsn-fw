/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "leds" bsp module.
 */

#include "stdint.h"
#include "leds.h"
#include "board.h"
#include "board_info.h"

/*
//=========================== defines =========================================
#define LEDS_PORT_ERROR 6
#define LEDS_PORT_DEBUG 6
#define LEDS_PORT_SYNC 7
#define LEDS_PORT_RADIO 7

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init(void)
{
  //enable clock for this peripheral
  CMU_ClockEnable(cmuClock_HFPER, TRUE);
  CMU_ClockEnable(cmuClock_GPIO, TRUE);

  //set the led pins to output.
  GPIO_PinModeSet(gpioPortF, LEDS_PORT_ERROR, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortF, LEDS_PORT_DEBUG, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortF, LEDS_PORT_SYNC, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortF, LEDS_PORT_RADIO, gpioModePushPull, 0);
}

// red
void leds_error_on(void) {
	GPIO_PinOutSet(gpioPortF, LEDS_PORT_ERROR);
}

void leds_error_off(void) {
	GPIO_PinOutClear(gpioPortF, LEDS_PORT_ERROR);
}

void leds_error_toggle(void) {
	GPIO_PinOutToggle(gpioPortF, LEDS_PORT_ERROR);
}

uint8_t leds_error_isOn(void) {
	return (uint8_t) (1 == GPIO_PinOutGet(gpioPortF, LEDS_PORT_ERROR));
}

// orange
void leds_sync_on(void) {
	GPIO_PinOutSet(gpioPortF, LEDS_PORT_SYNC);
}

void leds_sync_off(void) {
	GPIO_PinOutClear(gpioPortF, LEDS_PORT_SYNC);
}

void leds_sync_toggle(void) {
	GPIO_PinOutToggle(gpioPortF, LEDS_PORT_SYNC);
}

uint8_t leds_sync_isOn(void) {
	return (uint8_t) (1 == GPIO_PinOutGet(gpioPortF, LEDS_PORT_SYNC));
}

// green
void leds_radio_on(void) {
	GPIO_PinOutSet(gpioPortF, LEDS_PORT_RADIO);
}

void leds_radio_off(void) {
	GPIO_PinOutClear(gpioPortF, LEDS_PORT_RADIO);
}

void leds_radio_toggle(void) {
	GPIO_PinOutToggle(gpioPortF, LEDS_PORT_RADIO);
}

uint8_t leds_radio_isOn(void) {
	return (uint8_t) (1 == GPIO_PinOutGet(gpioPortF, LEDS_PORT_RADIO));
}

// yellow
void leds_debug_on(void) {
	GPIO_PinOutSet(gpioPortF, LEDS_PORT_DEBUG);
}

void leds_debug_off(void) {
	GPIO_PinOutClear(gpioPortF, LEDS_PORT_DEBUG);
}

void leds_debug_toggle(void) {
	GPIO_PinOutToggle(gpioPortF, LEDS_PORT_DEBUG);
}

uint8_t leds_debug_isOn(void) {
	return (uint8_t) (1 == GPIO_PinOutGet(gpioPortF, LEDS_PORT_DEBUG));
}

// all
void leds_all_on(void) {
	leds_radio_on();
	leds_sync_on();
	leds_debug_on();
	leds_error_on();
}

void leds_all_off(void) {
	leds_radio_off();
	leds_sync_off();
	leds_debug_off();
	leds_error_off();
}

void leds_all_toggle(void) {
	leds_radio_toggle();
	leds_sync_toggle();
	leds_debug_toggle();
	leds_error_toggle();
}

void leds_error_blink(void) {
	uint8_t i;
	volatile uint16_t delay;

	// turn all LEDs off
	leds_all_off();

	// blink error LED for ~10s
	for (i = 0; i < 80; i++) {
		leds_error_toggle();
		for (delay = 0xffff; delay > 0; delay--)
			;
		for (delay = 0xffff; delay > 0; delay--)
			;
	}
}

void leds_circular_shift(void) {
    //not implemented
	return;
}

void leds_increment(void) {
	//not implemented
	return;
}
*/
//=========================== private =========================================

