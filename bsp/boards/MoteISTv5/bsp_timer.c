/**
\brief MoteISTv5-specific definition of the "bsp_timer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
\author Diogo Guerra <diogoguerra@ist.utl.pt>, <dy090.guerra@gmail.com>, July 2015.
*/

#include "hal_MoteISTv5.h"
#include "bsp_timer.h"
#include "board.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   bsp_timer_cbt    cb;
   PORT_TIMER_WIDTH last_compare_value;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
\brief Initialize this module.

This functions starts the timer, i.e. the counter increments, but doesn't set
any compare registers, so no interrupt will fire.
*/
void bsp_timer_init() {
   
   // clear local variables
   memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));
   
   // set CCRA0 registers
   TB0CCTL0             =  0;
   TB0CCR0               =  0;
 
   
   //start TimerB0
   TB0CTL               =  MC_2 | TBSSEL_1;
}

/**
\brief Register a callback.

\param cb The function to be called when a compare event happens.
*/
void bsp_timer_set_callback(bsp_timer_cbt cb) {
   bsp_timer_vars.cb   = cb;
}

/**
\brief Reset the timer.

This function does not stop the timer, it rather resets the value of the
counter, and cancels a possible pending compare event.
*/
void bsp_timer_reset() {
   // reset compare
   TB0CCR0              =  0;
   TB0CCTL0             =  0;
   // reset timer
   TB0R                 = 0;
   // record last timer compare value
   bsp_timer_vars.last_compare_value =  0;
}

/**
\brief Schedule the callback to be called in some specified time.

The delay is expressed relative to the last compare event. It doesn't matter
how long it took to call this function after the last compare, the timer will
expire precisely delayTicks after the last one.

The only possible problem is that it took so long to call this function that
the delay specified is shorter than the time already elapsed since the last
compare. In that case, this function triggers the interrupt to fire right away.

This means that the interrupt may fire a bit off, but this inaccuracy does not
propagate to subsequent timers.

\param delayTicks Number of ticks before the timer expired, relative to the
                  last compare event.
*/
void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks) {
   PORT_TIMER_WIDTH newCompareValue;
   PORT_TIMER_WIDTH temp_last_compare_value;
   
   temp_last_compare_value = bsp_timer_vars.last_compare_value;
   
   newCompareValue      =  bsp_timer_vars.last_compare_value+delayTicks;
   bsp_timer_vars.last_compare_value   =  newCompareValue;
   
   if (delayTicks<TB0R-temp_last_compare_value) {
      // we're already too late, schedule the ISR right now, manually
      
      // setting the interrupt flag triggers an interrupt
      TB0CCTL0         |=  CCIE+CCIFG;
   } else {
      // this is the normal case, have timer expire at newCompareValue
      TB0CCR0            =  newCompareValue;
      TB0CCTL0         |=  CCIE;
   }
}

/**
\brief Cancel a running compare.
*/
void bsp_timer_cancel_schedule() {
   TB0CCR0               =  0;
   TB0CCTL0            &= ~CCIE;
}

/**
\brief Return the current value of the timer's counter.

\returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
   return TB0R;
}

//=========================== private =========================================

//=========================== interrup handlers ===============================

kick_scheduler_t bsp_timer_isr() {
   // call the callback
   bsp_timer_vars.cb();
   // kick the OS
   return KICK_SCHEDULER;
}
