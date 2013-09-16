#ifndef __LEDS_H
#define __LEDS_H

/**
\addtogroup BSP
\{
\addtogroup leds
\{

\brief Cross-platform declaration "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
 
//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void    leds_init();

void    leds_error_on();
void    leds_error_off();
void    leds_error_toggle();
uint8_t leds_error_isOn();
void    leds_error_blink();

void    leds_radio_on();
void    leds_radio_off();
void    leds_radio_toggle();
uint8_t leds_radio_isOn();

void    leds_sync_on();
void    leds_sync_off();
void    leds_sync_toggle();
uint8_t leds_sync_isOn();

void    leds_debug_on();
void    leds_debug_off();
void    leds_debug_toggle();
uint8_t leds_debug_isOn();

void    leds_all_on();
void    leds_all_off();
void    leds_all_toggle();

void    leds_circular_shift();
void    leds_increment();

/**
\}
\}
*/

#endif
