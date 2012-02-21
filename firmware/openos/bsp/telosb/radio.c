/**
\brief TelosB-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "radio.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void radio_init() {
   uint16_t delay;
   
   // VREG pin
   P4DIR     |=  0x20;                           // P4.5 radio VREG enabled, output
   P4OUT     |=  0x20;                           // P4.5 radio VREG enabled, hold high
   for (delay=0xffff;delay>0;delay--);           // max. VREG start-up time is 0.6ms
   
   // reset low
   P4DIR     |=  0x40;                           // P4.6 radio reset, output
   P4OUT     &= ~0x40;                           // P4.6 radio reset, hold low
   for (delay=0xffff;delay>0;delay--);
   
   // reset high
   P4OUT     |=  0x40;                           // P4.6 radio reset, hold high
   for (delay=0xffff;delay>0;delay--);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
