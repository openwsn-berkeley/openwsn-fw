/*
 * pins.h
 *
 *  Created on: Feb 24, 2012
 *      Author: nerd256
 */

#ifndef PINS_H_
#define PINS_H_

#include "internal/openal_internal_common.h"

typedef enum  {
	INPUT,
	OUTPUT,
	ANALOG_INPUT,
	ANALOG_OUTPUT,
	PWM
} pin_mode_t;

typedef enum  {
	LOW = 0,
	HIGH = 1
} pin_value_t;

typedef enum  {
	PULLUP = 0,
	PULLDOWN = 3,
	REPEATER = 1,
	NONE = 2
} pin_pullup_mode_t;

typedef enum  {
	NORMAL = 0,
	OPENDRAIN = 1
} pin_opendrain_mode_t;

typedef enum  {
	RISING,
	FALLING,
	BOTH
} pin_interrupt_mode_t;

void pins_pinMode(char pin, pin_mode_t mode) OPENAL_LIB ;
void pins_digitalWrite(char pin,pin_value_t value) OPENAL_LIB;

pin_value_t pins_digitalRead(char pin) OPENAL_LIB;
unsigned short pins_analogRead(char pin) OPENAL_LIB;
void pins_analogWrite(char pin, unsigned short value) OPENAL_LIB;

void pins_pwm_write(char pin, unsigned int duty_cycle) OPENAL_LIB;
void pins_pwm_write_adv(char pin, float duty_cycle, float frequency) OPENAL_LIB;

void pins_configure_pullups(char pin, pin_pullup_mode_t mode) OPENAL_LIB;
void pins_configure_opendrain(char pin, pin_opendrain_mode_t mode) OPENAL_LIB;

void pins_configure_interrupt(char pin, pin_interrupt_mode_t mode, void (*callback)(char pin, pin_interrupt_mode_t transition) ) OPENAL_LIB;
void pins_clear_interrupt(char pin) OPENAL_LIB;


#endif /* PINS_H_ */
