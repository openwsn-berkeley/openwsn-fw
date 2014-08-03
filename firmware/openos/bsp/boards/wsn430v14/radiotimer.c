/**
\brief WSN430v14-specific definition of the "radiotimer" bsp module.

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

void radiotimer_init(void) {
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
   
   // CCR0 contains period of counter
   // do not interrupt when counter reaches TACCR0, but when it resets
   TACCR0   =  period-1;
   
   // CCR1 in compare mode (disabled for now)
   TACCTL1  =  0;
   TACCR1   =  0;
   
   // start counting
   TACTL    =  TAIE+TACLR;                       // interrupt when counter resets
   TACTL   |=  MC_1+TASSEL_1;                    // up mode, clocked from ACLK
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue(void) {
   return TAR;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
   TACCR0   =  period;
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod(void) {
   return TACCR0;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
   // offset when to fire
   TACCR1   =  offset;
   
   // enable compare interrupt (this also cancels any pending interrupts)
   TACCTL1  =  CCIE;
}

void radiotimer_cancel(void) {
   // reset compare value (also resets interrupt flag)
   TACCR1   =  0;
   
   // disable compare interrupt
   TACCTL1 &= ~CCIE;
}

//===== capture

inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(void) {
   return TAR;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

/**
\brief TimerA CCR1-6 interrupt service routine
*/
kick_scheduler_t radiotimer_isr(void) {
   PORT_RADIOTIMER_WIDTH taiv_local;
   
   // reading TAIV returns the value of the highest pending interrupt flag
   // and automatically resets that flag. We therefore copy its value to the
   // tbiv_local local variable exactly once. If there is more than one 
   // interrupt pending, we will reenter this function after having just left
   // it.
   taiv_local = TAIV;
   
   switch (taiv_local) {
      case 0x0002: // CCR1 fires
         if (radiotimer_vars.compareCb!=NULL) {
            // call the callback
            radiotimer_vars.compareCb();
            // kick the OS
            return KICK_SCHEDULER;
         }
         break;
      case 0x0004: // CCR2 fires
         break;
      case 0x000a: // timer overflow
         if (radiotimer_vars.overflowCb!=NULL) {
            radiotimer_vars.overflowCb();
            // kick the OS
            return KICK_SCHEDULER;
         }
         break;
   }
   return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t radiotimer_isr_sfd(void) {
   uint16_t now;
   now = radiotimer_getCapturedTime();
   if (P1IES & 0x20) {
      // high->low just happened
      if (radiotimer_vars.endFrameCb!=NULL) {
         radiotimer_vars.endFrameCb(now);
      }
   } else {
      // low->high just happened
      if (radiotimer_vars.startFrameCb!=NULL) {
         radiotimer_vars.startFrameCb(now);
      }
   }
   P1IES     ^=  0x20;                     // arm in opposite transition
   return KICK_SCHEDULER;
}