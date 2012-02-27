/**
\brief GINA-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "stdio.h"
#include "string.h"
#include "radiotimer.h"

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflowCb;
   radiotimer_compare_cbt    compareCb;
   radiotimer_capture_cbt    startFrameCb;
   radiotimer_capture_cbt    endFrameCb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void radiotimer_init() {
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.overflowCb     = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.compareCb      = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_vars.startFrameCb = cb;
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_vars.endFrameCb   = cb;
}

void radiotimer_start(uint16_t period) {
   // source ACLK from 32kHz crystal
   BCSCTL3 |= LFXT1S_0;
   
   // CCR0 contains max value of counter (slot length)
   // do not interrupt when counter reaches TACCR0!
   TACCR0   =  period;
   
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

void radiotimer_schedule(uint16_t offset) {
   // offset when to fire
   TACCR1   =  offset;
   
   // enable CCR1 interrupt (this also cancels any pending interrupts)
   TACCTL1  =  CCIE;
}

void radiotimer_cancel() {
   // reset CCR1 value (also resets interrupt flag)
   TACCR1   =  0;
   
   // disable CCR1 interrupt
   TACCTL1 &= ~CCIE;
}

inline uint16_t radiotimer_getCapturedTime() {
   return TAR;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

/**
\brief TimerB CCR1-6 interrupt service routine
*/
#pragma vector=TIMERB1_VECTOR
__interrupt void timerb1_ISR (void) {
   uint16_t tbiv_local;
   
   // reading TBIV returns the value of the highest pending interrupt flag
   // and automatically resets that flags. We therefore copy its value to the
   // tbiv_local local variable exactly once. If there is more than one 
   // interrupt pending, we will reenter this function after having just left
   // it.
   tbiv_local = TBIV;
   
   switch (tbiv_local) {
      case 0x0002: // CCR1 fires
         if (TBCCTL1 & CCI) {
            // SFD pin is high: this was the start of a frame
            if (radiotimer_vars.startFrameCb!=NULL) {
               radiotimer_vars.startFrameCb(TBCCR1);
            }
         } else {
            // SFD pin is low: this was the end of a frame
            if (radiotimer_vars.endFrameCb!=NULL) {
               radiotimer_vars.endFrameCb(TBCCR1);
            }
         }
         break;
      case 0x0004: // CCR2 fires
         if (radiotimer_vars.compareCb!=NULL) {
            radiotimer_vars.compareCb();
         }
         break;
      case 0x0006: // CCR3 fires
         break;
      case 0x0008: // CCR4 fires
         break;
      case 0x000a: // CCR5 fires
         break;
      case 0x000c: // CCR6 fires
         break;
      case 0x000e: // timer overflow
         if (radiotimer_vars.overflowCb!=NULL) {
            radiotimer_vars.overflowCb();
         }
         break;
   }
   
   __bic_SR_register_on_exit(CPUOFF);  // restart CPU
}
