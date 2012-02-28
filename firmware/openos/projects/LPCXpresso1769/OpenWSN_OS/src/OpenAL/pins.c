/*
 * pins.c
 *
 *  Created on: Feb 24, 2012
 *      Author: nerd256
 */


#include "pins.h"
#include "internal/openal_app_manager.h"
#include "internal/openmote_pindefs.h"

char buf[128];

void pins_pinMode(char pin, pin_mode_t mode)  {
	if ( mode >= INPUT && mode <= PWM ){
		if (openal_claim_pin(pin) == SUCCESS) {
			openal_pin_registry[pin-1].mode = mode;
			switch(mode) {


			}
		} else goto error;
	} else goto error;
	return;
	error: // app will die for its atrocities
		openal_kill_app(APP_CURRENT);

}

void pins_digitalWrite(char pin,pin_value_t value) {
	pin += 1;
}

pin_value_t pins_digitalRead(char pin) {
	pin += 1;
}

unsigned short pins_analogRead(char pin) {
	return buf[pin];
}

void pins_analogWrite(char pin, unsigned short value) {
	pin += 1;
}

void pins_pwm_write(char pin, unsigned int duty_cycle) {
	pin += 1;
}

void pins_pwm_write_adv(char pin, unsigned int duty_cycle, unsigned int frequency) {
	buf[pin]++;
}

void pins_configure_pullups(char pin, pin_pullup_mode_t mode) {
	pin += 1;
}

void pins_configure_opendrain(char pin, pin_opendrain_mode_t mode) {
	pin += 1;
}

void pins_configure_interrupt(char pin, pin_interrupt_mode_t mode, void (*callback)(char pin, pin_interrupt_mode_t transition) ) {
	pin += 1;
}

void pins_clear_interrupt(char pin) {
	pin += 1;
}

