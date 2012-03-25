/**
\brief TelosB-specific definition of the "bsp_timer" bsp module.

On TelosB, we use timerA0 for the bsp_timer module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "msp430f1611.h"
#include "string.h"
#include "bsp_timer.h"
#include "board.h"
#include "board_info.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   bsp_timer_cbt cb;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void bsp_timer_init() {
   
   // clear local variables
   memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));
   
   // set CCRA0 registers
   TACCR0               =  0;
   TACCTL0              =  0;
   
   //start TimerA
   TACTL                =  MC_2+TASSEL_1;        // continuous mode, from ACLK
}

void bsp_timer_set_callback(bsp_timer_cbt cb) {
   bsp_timer_vars.cb   = cb;
}

void bsp_timer_set_compare(uint16_t compareValue) {
   TACCR0               =  compareValue;
   TACCTL0             |=  CCIE;
}

void bsp_timer_cancel_compare() {
   TACCR0               =  0;
   TACCTL0             &= ~CCIE;
}

PORT_TIMER_WIDTH bsp_timer_get_current_value() {
   return TAR;
}

//=========================== private =========================================

//=========================== interrup handlers ===============================

uint8_t bsp_timer_isr() {
   // call the callback
   bsp_timer_vars.cb();
   // kick the OS
   return 1;
}