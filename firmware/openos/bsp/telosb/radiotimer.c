/**
\brief TelosB-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "radiotimer.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void radiotimer_init(uint16_t period) {
   /*
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
   */
}

void radiotimer_schedule(uint16_t offset) {
   /*
   // offset when to fire
   TACCR1   =  offset;
   // enable CCR1 interrupt (this also cancels any pending interrupts)
   TACCTL1  =  CCIE;
   */
}

void radiotimer_cancel() {
   /*
   TACCR1   =  0;                                // reset CCR1 value (also resets interrupt flag)
   TACCTL1 &= ~CCIE;                             // disable CCR1 interrupt
   */
}

inline uint16_t radiotimer_getCapturedTime() {
   /*
   return TAR;
   */
   return 0;
}

//=========================== private =========================================
