#ifndef __LEDS_H
#define __LEDS_H

/**
\addtogroup drivers
\{
\addtogroup Leds
\{

On TelosB, the LEDs are connected to:
   - P5.4: red LED   -> error
   - P5.5: green LED -> radio
   - P5.6: blue LED  -> sync
*/

#include "msp430f1611.h"
#include "stdint.h"
 
//=========================== define ==========================================

#define LED_ERROR_ON()       P5OUT |=  0x10; // red
#define LED_ERROR_OFF()      P5OUT &= ~0x10;
#define LED_ERROR_TOGGLE()   P5OUT ^=  0x10;

#define LED_RADIO_ON()       P5OUT |=  0x20; // green
#define LED_RADIO_OFF()      P5OUT &= ~0x20;
#define LED_RADIO_TOGGLE()   P5OUT ^=  0x20;

#define LED_SYNC_ON()        P5OUT |=  0x40; // blue
#define LED_SYNC_OFF()       P5OUT &= ~0x40;
#define LED_SYNC_TOGGLE()    P5OUT ^=  0x40;

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void leds_init();
void leds_circular_shift();
void leds_increment();

/**
\}
\}
*/

#endif
