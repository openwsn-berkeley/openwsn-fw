#ifndef __LEDS_H
#define __LEDS_H

/**
\addtogroup drivers
\{
\addtogroup Leds
\{
*/

#include "stdint.h"
 
//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void leds_init();
void leds_circular_shift();
void leds_increment();
void leds_blink();
void leds_setCombination(uint8_t combination);

void led_radioLedOn();
void led_radioLedOff();
void led_radioLedToggle();
void led_syncLedOn();
void led_syncLedOff();
void led_syncLedToggle();
void led_errorLedOn();
void led_errorLedOff();
void led_errorLedToggle();

/**
\}
\}
*/

#endif
