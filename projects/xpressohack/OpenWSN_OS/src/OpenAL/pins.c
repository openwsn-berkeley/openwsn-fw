/*
 * pins.c
 *
 *  Created on: Feb 24, 2012
 *      Author: nerd256
 */


#include "pins.h"
#include "internal/openal_app_manager.h"
#include "internal/openal_event_manager.h"
#include "internal/openmote_pindefs.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

void pins_pinMode(char pin, pin_mode_t mode)  {
	portBASE_TYPE xRunningPrivileged;
	char c;
	unsigned long pwm_on;
	// Make sure pin is valid
	xSemaphoreTake(pin_configuration_mutex, portMAX_DELAY);

	// pin is 1-indexed
	pin--;

	if ( pin < 0 || pin >= OPENMOTE_NUM_PINS || openmote_pin_gpio_bit[pin] == NOT_VALID_PIN ) goto error;

	// see if pin choice is supported
	switch(mode) {
		case INPUT: // already checked
		case OUTPUT:
			break;
		case ANALOG_INPUT:
			if ( openmote_adc_channels[pin] == NOT_VALID_PIN ) goto error;
			break;
		case ANALOG_OUTPUT:
			if ( DAC_AOUT != pin ) goto error;
			break;
		case PWM:
			if ( openmote_pwm_channels[pin] == NOT_VALID_PIN ) goto error;
			break;
		default:
			goto error;
	}

	// Valid pin, now see if we can claim it
	if (openal_claim_pin(pin + 1) == SUCCESS) {
		// see if we even need to configure anything
		if ( openal_pin_registry[pin].mode == mode ) {
			goto success;
		}

		// we now need privileges!
		xRunningPrivileged = prvRaisePrivilege();

		// switch out of the previous mode
		switch(openal_pin_registry[pin].mode) {
			case INPUT:
			case OUTPUT:
			case ANALOG_INPUT:
			case ANALOG_OUTPUT: // power turns on and off automatically with PINSEL
				break; // nothing we need to do
			case PWM:
				// turn off pwm pin
				LPC_PWM1->PCR &= ~(1<<(openmote_pwm_channels[pin] + 9));
				// check to see if any other pin was in PWM mode
				for (c = 0; c < OPENMOTE_NUM_PINS; c++ ) {
					if ( c == pin ) continue;
					if ( openal_pin_registry[c].mode == PWM )
						break;
				}
				if ( c == OPENMOTE_NUM_PINS ) { // turn off PWM module
					LPC_PWM1->TCR = 0;
					LPC_SC->PCONP &= ~(1<<6);
				}
			default:
				break;
		}

		openal_pin_registry[pin].mode = mode;
		switch(mode) {
			case INPUT:
				LPC17xx_gpio_bank_addresses[openmote_pin_gpio_bank[pin]]->FIODIR &= ~(1<<openmote_pin_gpio_bit[pin]);
				openmote_pin_function_select(openmote_pin_gpio_bank[pin],openmote_pin_gpio_bit[pin],0);
				break;
			case OUTPUT:
				LPC17xx_gpio_bank_addresses[openmote_pin_gpio_bank[pin]]->FIODIR |= (1<<openmote_pin_gpio_bit[pin]);
				openmote_pin_function_select(openmote_pin_gpio_bank[pin],openmote_pin_gpio_bit[pin],0);
				break;
			case ANALOG_INPUT:
				openmote_pin_function_select(openmote_pin_gpio_bank[pin],openmote_pin_gpio_bit[pin], openmote_adc_pinsel_value[pin]);
			case ANALOG_OUTPUT:
				openmote_pin_function_select(openmote_pin_gpio_bank[pin],openmote_pin_gpio_bit[pin],DAC_PINSEL_VALUE);
				pins_configure_pullups(pin+1, NONE);
				pins_analogWrite(pin + 1, 0); // ends our privilege
			case PWM:
				// turn on PWM module if it is not already
				pwm_on = (LPC_SC->PCONP & (1<<6));
				if ( pwm_on == 0 ) {
					LPC_SC->PCONP |= (1<<6);
					LPC_PWM1->TCR = 0x09; // Turn counters on
				}
				openmote_pin_function_select(openmote_pin_gpio_bank[pin],openmote_pin_gpio_bit[pin], openmote_pwm_pinsel_value[pin]);
				pins_pwm_write_adv(pin + 1, 0, 490); // ends our privileges
			default:
				break;
		}

		// reset privileges!
		portRESET_PRIVILEGE(xRunningPrivileged);

		goto success;
	} else goto error;

	success:
		xSemaphoreGive(pin_configuration_mutex);
		return;

	error: // app will die for its atrocities
		xSemaphoreGive(pin_configuration_mutex);
		openal_kill_app(APP_CURRENT);
		// shouldn't get to here
}

void pins_digitalWrite(char pin,pin_value_t value) {
	portBASE_TYPE xRunningPrivileged;
	if ( openal_check_owner(pin) == SUCCESS ) {
		pin--;

		xRunningPrivileged = prvRaisePrivilege();
		if (value)
			LPC17xx_gpio_bank_addresses[openmote_pin_gpio_bank[pin]]->FIOSET = (1<<openmote_pin_gpio_bit[pin]);
		else
			LPC17xx_gpio_bank_addresses[openmote_pin_gpio_bank[pin]]->FIOCLR = (1<<openmote_pin_gpio_bit[pin]);
		portRESET_PRIVILEGE(xRunningPrivileged);
	} else {
		openal_kill_app(APP_CURRENT);
	}
}

pin_value_t pins_digitalRead(char pin) {
	portBASE_TYPE xRunningPrivileged;
	pin_value_t retval;
	if ( openal_check_owner(pin) == SUCCESS ) {
		pin--;

		xRunningPrivileged = prvRaisePrivilege();
		retval  = (LPC17xx_gpio_bank_addresses[openmote_pin_gpio_bank[pin]]->FIOPIN & (1<<openmote_pin_gpio_bit[pin]))?HIGH:LOW;
		portRESET_PRIVILEGE(xRunningPrivileged);

		return retval;
	} else {
		openal_kill_app(APP_CURRENT);
		return LOW; // shouldn't get here
	}
}

unsigned short pins_analogRead(char pin) {
	portBASE_TYPE xRunningPrivileged;
	unsigned short retval;
	if ( openal_check_owner(pin) == SUCCESS ) {
		pin--;

		xRunningPrivileged = prvRaisePrivilege();
		xSemaphoreTake(adc_mutex, portMAX_DELAY);
		LPC_SC->PCONP |= (1<<12); // enable ADC peripheral power

		xSemaphoreTake(adc_conversion_semaphore, 0); // make sure the semaphore is 0
		LPC_ADC->ADINTEN = (1<<8); // interrupt on global DONE flag
		LPC_ADC->ADCR = openmote_adc_channels[pin] | // channel
						((7) << 8) | // clock div 100/8 = 12.5MHz
						(1 << 21) | // Power ON
						(1 << 24); // start conversion NOW

		xSemaphoreTake(adc_conversion_semaphore, portMAX_DELAY); // wait for conversion to finish

		retval = ((LPC_ADC->ADGDR) >> 4) & 0x0FFF;

		LPC_ADC->ADCR = 0; // Disable ADC
		LPC_SC->PCONP &= ~(1<<12); // enable ADC peripheral power
		xSemaphoreGive(adc_mutex);
		portRESET_PRIVILEGE(xRunningPrivileged);

		return retval;
	} else {
		openal_kill_app(APP_CURRENT);
		return 0; // shouldn't get here
	}
}

void pins_analogWrite(char pin, unsigned short value) {
	portBASE_TYPE xRunningPrivileged;
	if (openal_check_owner(pin) == SUCCESS) {
		value &= 1023;// only allow 10-bit value

		xRunningPrivileged = prvRaisePrivilege();
		LPC_DAC->DACR = (value<<6) | (1<<16); // Add BIAS bit for lower power (350uA)
		portRESET_PRIVILEGE(xRunningPrivileged);
	} else {
		openal_kill_app(APP_CURRENT);
	}
}

void pins_pwm_write(char pin, unsigned int duty_cycle) {
	char channel;
	portBASE_TYPE xRunningPrivileged;
	if ( openal_check_owner(pin) == SUCCESS ) {
		xRunningPrivileged = prvRaisePrivilege();

		channel = openmote_pwm_channels[pin-1];
		*openmote_pwm_match_registers[channel] = (unsigned int)(LPC_PWM1->MR6 * duty_cycle + 0.5);

		portRESET_PRIVILEGE(xRunningPrivileged);
	} else {
		openal_kill_app(APP_CURRENT);
	}
}

void pins_pwm_write_adv(char pin, float duty_cycle, float frequency) {
	portBASE_TYPE xRunningPrivileged;
	char channel;
	if ( openal_check_owner(pin) == SUCCESS ) {
		xRunningPrivileged = prvRaisePrivilege();

		// Using match register 6 to determine period, no prescaler (already at CLK/4 from peripheral clock)
		LPC_PWM1->PR = 0;
		LPC_PWM1->MR6 = (int)(f_cpu_awake/4/frequency);
		LPC_PWM1->MCR = (1<<19);

		channel = openmote_pwm_channels[pin-1];
		*openmote_pwm_match_registers[channel] = (unsigned int)(LPC_PWM1->MR6 * duty_cycle + 0.5);

		portRESET_PRIVILEGE(xRunningPrivileged);
	} else {
		openal_kill_app(APP_CURRENT);
	}
}

void pins_configure_pullups(char pin, pin_pullup_mode_t mode) {
	portBASE_TYPE xRunningPrivileged;
	if ( mode >= 0 && mode <= 3 && openal_check_owner(pin) == SUCCESS ) {
		xRunningPrivileged = prvRaisePrivilege();
		openmote_pin_mode_select(openmote_pin_gpio_bank[pin],openmote_pin_gpio_bit[pin],mode);
		portRESET_PRIVILEGE(xRunningPrivileged);
	} else {
		openal_kill_app(APP_CURRENT);
	}
}

void pins_configure_opendrain(char pin, pin_opendrain_mode_t mode) {
	portBASE_TYPE xRunningPrivileged;
	if ( mode >= 0 && mode <= 1 && openal_check_owner(pin) == SUCCESS ) {
		xRunningPrivileged = prvRaisePrivilege();
		openmote_pin_open_drain_select(openmote_pin_gpio_bank[pin],openmote_pin_gpio_bit[pin],mode);
		portRESET_PRIVILEGE(xRunningPrivileged);
	} else {
		openal_kill_app(APP_CURRENT);
	}
}

void pins_configure_interrupt(char pin, pin_interrupt_mode_t mode, void (*callback)(char pin, pin_interrupt_mode_t transition) ) {
	portBASE_TYPE xRunningPrivileged;
	char cur_app, port, gp_pin;
	if ( openal_check_owner(pin) == SUCCESS && (cur_app = openal_current_app()) != APP_NOT_FOUND ) {
		pin--; // 1-index fix
		port = openmote_pin_gpio_bank[pin];
		gp_pin = openmote_pin_gpio_bit[pin];
		if ( port != 0 && port != 2) { // not an interrupt-capable pin!
			openal_kill_app(APP_CURRENT);
			return;
		}
		xRunningPrivileged = prvRaisePrivilege();
		openal_GPIO_Listeners[pin] = cur_app;
		openal_GPIO_Listener_type[pin] = mode;

		if (mode == RISING || mode == BOTH ) {
			if ( port == 2 )
				LPC_GPIOINT->IO2IntEnR |= (1<<gp_pin);
			else
				LPC_GPIOINT->IO0IntEnR |= (1<<gp_pin);
		}
		if (mode == FALLING || mode == BOTH ) {
			if ( port == 2 )
				LPC_GPIOINT->IO2IntEnF |= (1<<gp_pin);
			else
				LPC_GPIOINT->IO0IntEnF |= (1<<gp_pin);
		}

		portRESET_PRIVILEGE(xRunningPrivileged);
	} else {
		openal_kill_app(APP_CURRENT);
	}
}

void pins_clear_interrupt(char pin) {
	portBASE_TYPE xRunningPrivileged;
	char port, gp_pin;
	if ( openal_check_owner(pin) == SUCCESS ) {
		pin--; // 1-index fix
		port = openmote_pin_gpio_bank[pin];
		gp_pin = openmote_pin_gpio_bit[pin];
		if ( port != 0 && port != 2) { // not an interrupt-capable pin!
			openal_kill_app(APP_CURRENT);
			return;
		}

		xRunningPrivileged = prvRaisePrivilege();
		openal_GPIO_Listeners[pin] = APP_SLOT_EMPTY;
		openal_GPIO_Listener_type[pin] = APP_SLOT_EMPTY;

		if ( port == 2 ) {
			LPC_GPIOINT->IO2IntEnR &= ~(1<<gp_pin);
			LPC_GPIOINT->IO2IntEnF &= ~(1<<gp_pin);
		}
		else
			LPC_GPIOINT->IO0IntEnR &= ~(1<<gp_pin);{
			LPC_GPIOINT->IO0IntEnF &= ~(1<<gp_pin);
		}

		portRESET_PRIVILEGE(xRunningPrivileged);
	} else {
		openal_kill_app(APP_CURRENT);
	}
}

