/**
\brief TelosB-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "radiotimer.h"
#include "leds.h"

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

//===== admin

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
   radiotimer_vars.startFrameCb   = cb;
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_vars.endFrameCb     = cb;
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
   // radio's SFD pin connected to P4.1
   P4DIR   &= ~0x02; // input
   P4SEL   |=  0x02; // in CCI1a/B mode
   
   // CCR0 contains period of counter
   // do not interrupt when counter reaches TBCCR0, but when it resets
   TBCCR0   =  period-1;
   
   // CCR1 in capture mode
   TBCCTL1  =  CM_3+SCS+CAP+CCIE;
   TBCCR1   =  0;
   
   // CCR2 in compare mode (disabled for now)
   TBCCTL2  =  0;
   TBCCR2   =  0;
   
   // start counting
   TBCTL    =  TBIE+TBCLR;                       // interrupt when counter resets
   TBCTL   |=  MC_1+TBSSEL_1;                    // up mode, clocked from ACLK
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
   return TBR;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
   TBCCR0   =  period;
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
   return TBCCR0;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
   // offset when to fire
   TBCCR2   =  offset;
   
   // enable compare interrupt (this also cancels any pending interrupts)
   TBCCTL2  =  CCIE;
}

void radiotimer_cancel() {
   // reset compare value (also resets interrupt flag)
   TBCCR2   =  0;
   
   // disable compare interrupt
   TBCCTL2 &= ~CCIE;
}

//===== capture

inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
   // this should never happpen!
   
   // we can not print from within the BSP. Instead:
   // blink the error LED
   leds_error_blink();
   // reset the board
   board_reset();
   
   return 0;// this line is never reached, but here to satisfy compiler
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

/**
\brief TimerB CCR1-6 interrupt service routine
*/
kick_scheduler_t radiotimer_isr() {
   PORT_RADIOTIMER_WIDTH tbiv_local;
   
   // reading TBIV returns the value of the highest pending interrupt flag
   // and automatically resets that flag. We therefore copy its value to the
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
               // kick the OS
               return KICK_SCHEDULER;
            }
         } else {
            // SFD pin is low: this was the end of a frame
            if (radiotimer_vars.endFrameCb!=NULL) {
               radiotimer_vars.endFrameCb(TBCCR1);
               // kick the OS
               return KICK_SCHEDULER;
            }
         }
         break;
      case 0x0004: // CCR2 fires
         if (radiotimer_vars.compareCb!=NULL) {
            radiotimer_vars.compareCb();
            // kick the OS
            return KICK_SCHEDULER;
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
            // kick the OS
            return KICK_SCHEDULER;
         }
         break;
   }
   return DO_NOT_KICK_SCHEDULER;
}
