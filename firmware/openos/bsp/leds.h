/**
\brief Cross-platform declaration "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __LEDS_H
#define __LEDS_H
 
//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void leds_init();

void led_error_on();
void led_error_off();
void led_error_toggle();

void led_radio_on();
void led_radio_off();
void led_radio_toggle();

void led_sync_on();
void led_sync_off();
void led_sync_toggle();

void led_all_on();
void led_all_off();
void led_all_toggle();

void leds_circular_shift();
void leds_increment();

#endif
