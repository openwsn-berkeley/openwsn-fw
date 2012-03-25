/**
\brief GINA-specific definition of the "timers" bsp module.

On GINA, we use timerB0 for the bsp_timer module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "msp430x26x.h"
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
   
   // source ACLK from 32kHz crystal
   BCSCTL3             |=  LFXT1S_0;

   //set CCRB0 registers
   TBCCTL0              =  0;
   TBCCR0               =  0;
   
   //start TimerB
   TBCTL                =  MC_2+TBSSEL_1;             // continuous mode, from ACLK
}

void bsp_timer_set_callback(bsp_timer_cbt cb) {
   bsp_timer_vars.cb   = cb;
}

void bsp_timer_set_compare(uint16_t compareValue) {
   TBCCR0               =  compareValue;
   TBCCTL0             |=  CCIE;
}

void bsp_timer_cancel_compare() {
   TBCCR0               =  0;
   TBCCTL0             &= ~CCIE;
}

PORT_TIMER_WIDTH bsp_timer_get_current_value() {
   return TBR;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

uint8_t bsp_timer_isr() {
   // call the callback
   bsp_timer_vars.cb();
   // kick the OS
   return 1;
}