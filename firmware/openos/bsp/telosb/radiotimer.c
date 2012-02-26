/**
\brief TelosB-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
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
   // radio's SFD pin connected to P4.1
   P4DIR   &= ~0x02; // input
   P4SEL   |=  0x02; // in CCI1a/B mode
   
   // CCR0 contains period of counter
   // do not interrupt when counter reaches TBCCR0, but when it resets
   TBCCR0   =  period;
   
   // CCR1 in capture mode
   TBCCTL1  =  CM_3+SCS+CAP+CCIE;
   TBCCR1   =  0;
   
   // CCR2 in compare mode
   TBCCTL2  =  0;
   TBCCR2   =  0;
   
   // start counting
   TBCTL    =  TBIE+TBCLR;                       // interrupt when counter resets
   TBCTL   |=  MC_1+TBSSEL_1;                    // up mode, clocked from ACLK
}

void radiotimer_schedule(uint16_t offset) {
   // offset when to fire
   TACCR2   =  offset;
   
   // enable compare interrupt (this also cancels any pending interrupts)
   TACCTL2  =  CCIE;
}

void radiotimer_cancel() {
   // reset compare value (also resets interrupt flag)
   TACCR2   =  0;
   
   // disable compare interrupt
   TACCTL2 &= ~CCIE;
}

inline uint16_t radiotimer_getCapturedTime() {
   return TACCR1;
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


