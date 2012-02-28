/*
 * pins.c
 *
 *  Created on: Feb 24, 2012
 *      Author: nerd256
 */


#include "pins.h"
#include "internal/openal_app_manager.h"
#include "internal/openmote_pindefs.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"


void pins_pinMode(char pin, pin_mode_t mode)  {
	// Make sure pin is valid
	if ( pin < 1 || pin > OPENMOTE_NUM_PINS ) goto error;


	xSemaphoreTake(pin_configuration_mutex, portMAX_DELAY);
	if ( mode >= INPUT && mode <= PWM ){
		if (openal_claim_pin(pin) == SUCCESS) {
			if ( openal_pin_registry[pin-1].mode == mode ) { // already in the correct mode
				goto success;
			}

			// switch out of the previous mode
			switch(openal_pin_registry[pin-1].mode) {
				case INPUT:
				case OUTPUT:
				case ANALOG_INPUT:
				case ANALOG_OUTPUT:
				case PWM:
				default:
					break;
			}

			openal_pin_registry[pin-1].mode = mode;
			switch(mode) {
				case INPUT:
				case OUTPUT:
				case ANALOG_INPUT:
				case ANALOG_OUTPUT:
				case PWM:
				default:
					break;
			}
		} else goto error;
	} else goto error;

	success:
		xSemaphoreGive(pin_configuration_mutex);
		return;

	error: // app will die for its atrocities
		xSemaphoreGive(pin_configuration_mutex);
		openal_kill_app(APP_CURRENT);


}

void pins_digitalWrite(char pin,pin_value_t value) {

}

pin_value_t pins_digitalRead(char pin) {

}

unsigned short pins_analogRead(char pin) {
}

void pins_analogWrite(char pin, unsigned short value) {
}

void pins_pwm_write(char pin, unsigned int duty_cycle) {

}

void pins_pwm_write_adv(char pin, unsigned int duty_cycle, unsigned int frequency) {

}

void pins_configure_pullups(char pin, pin_pullup_mode_t mode) {

}

void pins_configure_opendrain(char pin, pin_opendrain_mode_t mode) {

}

void pins_configure_interrupt(char pin, pin_interrupt_mode_t mode, void (*callback)(char pin, pin_interrupt_mode_t transition) ) {

}

void pins_clear_interrupt(char pin) {

}

