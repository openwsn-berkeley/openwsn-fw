#include "msp430x26x.h"
#include "ieee154etimer.h"
#include "IEEE802154e.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
\brief Initialize the IEEE802.15.4e timer.
*/
void ieee154etimer_init() {
   
   // source ACLK from 32kHz crystal
   BCSCTL3 |= LFXT1S_0;
   
   // CCR0 contains max value of counter (slot length)
   // do not interrupt when counter reaches TACCR0!
   TACCR0   =  TsSlotDuration;
   
   // CCR1 in compare mode
   TACCTL1  =  0;
   TACCR1   =  0;
   
   // CCR2 in capture mode
   TACCTL2  =  CAP+SCS+CCIS1+CM_1;
   TACCR2   =  0;
   
   // reset couter
   TAR      =  0;
   
   // start counting
   TACTL    =  TAIE;                             // interrupt when counter resets
   TACTL   |=  MC_1+TBSSEL_1;                    // up mode, clocked from ACLK
}

//--- CCR1 compare timer

/**
\brief Schedule the timer to fire in the future.

Calling this function cancels all running timers

\param [in] offset The time at which to fire, relative to the current slot start
                   time, in 32-kHz ticks.
*/
void ieee154etimer_schedule(uint16_t offset) {
   // offset when to fire
   TACCR1   =  offset;
   // enable CCR1 interrupt (this also cancels any pending interrupts)
   TACCTL1  =  CCIE;
}

/**
\brief Cancel the timer.

This disables the interrupt associated with the TACCR1 compare register.

This function has no effect if no timer is running.
*/
void ieee154etimer_cancel() {
   TACCR1   =  0;                                // reset CCR1 value (also resets interrupt flag)
   TACCTL1 &= ~CCIE;                             // disable CCR1 interrupt
}

//--- CCR2 capture timer
inline uint16_t ieee154etimer_getCapturedTime() {
   return TAR;
}

//=========================== private =========================================
