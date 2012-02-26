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

void leds_error_on();
void leds_error_off();
void leds_error_toggle();

void leds_radio_on();
void leds_radio_off();
void leds_radio_toggle();

void leds_sync_on();
void leds_sync_off();
void leds_sync_toggle();

void leds_all_on();
void leds_all_off();
void leds_all_toggle();

void leds_circular_shift();
void leds_increment();

#endif
