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
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
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
   radiotimer_vars.overflow_cb    = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.compare_cb     = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   while(1);
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   while(1);
}

void radiotimer_start(uint16_t period) {
   // source ACLK from 32kHz crystal
   BCSCTL3 |= LFXT1S_0;
   
   // CCR0 contains max value of counter (slot length)
   // do not interrupt when counter reaches TACCR0!
   TACCR0   =  period;
   
   // CCR1 in compare mode (disabled for now)
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

//===== direct access

uint16_t radiotimer_getValue() {
   return TAR;
}

void radiotimer_setPeriod(uint16_t period) {
   TACCR0   =  period;
}

uint16_t radiotimer_getPeriod() {
   return TACCR0;
}

//===== compare

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

//===== capture

inline uint16_t radiotimer_getCapturedTime() {
   return TAR;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

uint8_t radiotimer_isr() {
   uint16_t taiv_temp = TAIV;                    // read only once because accessing TAIV resets it
   switch (taiv_temp) {
      case 0x0002: // capture/compare CCR1
         if (radiotimer_vars.compare_cb!=NULL) {
            // call the callback
            radiotimer_vars.compare_cb();
            // kick the OS
            return 1;
         }
         break;
      case 0x000a: // timer overflows
         if (radiotimer_vars.overflow_cb!=NULL) {
            // call the callback
            radiotimer_vars.overflow_cb();
            // kick the OS
            return 1;
         }
         break;
      case 0x0004: // capture/compare CCR2
      default:
         while(1);                               // this should not happen
   }
   return 0;
}