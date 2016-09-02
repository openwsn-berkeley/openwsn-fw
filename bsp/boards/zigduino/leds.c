/**
\brief Zigduino definition of the "leds" bsp module.

\author Sven Akkermans <sven.akkermans@cs.kuleuven.be>, September 2015.
 */

#include <avr/io.h>

#include "leds.h"

//=========================== defines =========================================
/*
 * PORT where LEDs are connected
 */
#define LED_PORT0                        (PORTB)
#define LED_PORT_DIR0                    (DDRB)
#define LED_PORT                        (PORTD)
#define LED_PORT_DIR                    (DDRD)

/*
 * PINs where LEDs are connected
 */
#define LED_PIN_13                       (PB1)  //A free led, we use it for sync and debug
#define LED_ERR_1                       (PD5) //first red error led, RFRX
#define LED_ERR_2                       (PD6) //second red error led, RFTX

#define LED_TX

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void    leds_init() {
	LED_PORT0 |= (1 << LED_PIN_13);
	LED_PORT_DIR0 |= (1 << LED_PIN_13);
	LED_PORT |= (1 << LED_ERR_1);
	LED_PORT_DIR |= (1 << LED_ERR_1);
	LED_PORT |= (1 << LED_ERR_2);
	LED_PORT_DIR |= (1 << LED_ERR_2);

	leds_all_off();
}

void    leds_error_on() {
	LED_PORT |= (1 << LED_ERR_1);
	LED_PORT_DIR |= (1 << LED_ERR_1);
}

void    leds_error_off() {
	LED_PORT &= ~(1 << LED_ERR_1);
	LED_PORT_DIR |= (1 << LED_ERR_1);
}

void    leds_error_toggle() {
	LED_PORT ^= (1 << LED_ERR_1);
	LED_PORT_DIR |= (1 << LED_ERR_1);
}

uint8_t leds_error_isOn() {
	return 0;
}
void leds_error_blink() {
}

void    leds_radio_on() {
	LED_PORT |= (1 << LED_ERR_2);
	LED_PORT_DIR |= (1 << LED_ERR_2); //transmission led
}
void    leds_radio_off() {
	LED_PORT &= ~(1 << LED_ERR_2);
	LED_PORT_DIR |= (1 << LED_ERR_2);
}
void    leds_radio_toggle() {
	LED_PORT ^= (1 << LED_ERR_2);
	LED_PORT_DIR |= (1 << LED_ERR_2);
}
uint8_t leds_radio_isOn() {
	return 0;
}

void    leds_sync_on() {
	LED_PORT0 |= (1 << LED_PIN_13);
	LED_PORT_DIR0 |= (1 << LED_PIN_13);
}
void    leds_sync_off() {
	LED_PORT0 &= ~(1 << LED_PIN_13);
	LED_PORT_DIR0 |= (1 << LED_PIN_13);

}
void    leds_sync_toggle() {
	LED_PORT0 ^= (1 << LED_PIN_13);
	LED_PORT_DIR0 |= (1 << LED_PIN_13);
}
uint8_t leds_sync_isOn() {
	return 0;
}

void    leds_debug_on() {
	LED_PORT0 |= (1 << LED_PIN_13);
	LED_PORT_DIR0 |= (1 << LED_PIN_13);
}
void    leds_debug_off() {
	LED_PORT0 &= ~(1 << LED_PIN_13);
	LED_PORT_DIR0 |= (1 << LED_PIN_13);
}
void    leds_debug_toggle() {
	LED_PORT0 ^= (1 << LED_PIN_13);
	LED_PORT_DIR0 |= (1 << LED_PIN_13);
}
uint8_t leds_debug_isOn() {
	return 0;
}

void    leds_all_on() {
	leds_error_on();
	leds_radio_on();
	leds_sync_on();
}
void    leds_all_off() {
	leds_error_off();
	leds_radio_off();
	leds_sync_off();
}
void    leds_all_toggle() {
	leds_error_toggle();
	leds_radio_toggle();
	leds_sync_toggle();
}

void    leds_circular_shift() {
}

void    leds_increment() {
}
//=========================== private =========================================
