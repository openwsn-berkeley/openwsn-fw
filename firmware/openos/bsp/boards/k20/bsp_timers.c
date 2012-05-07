/**
\brief K20-specific definition of the "timers" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, April 2012.
*/

#include "bsp_timer.h"
#include "board.h"
#include "board_info.h"
#include "lptmr.h"


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
   lptmr_init(LPTMR_USE_ERCLK32);
   lptmr_set_isr_compare_hook(bsp_timer_isr);
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
   PORT_TIMER_WIDTH current_value;

   temp_last_compare_value = bsp_timer_vars.last_compare_value;

   newCompareValue      =  bsp_timer_vars.last_compare_value+delayTicks;
   //newCompareValue      =  delayTicks;
   bsp_timer_vars.last_compare_value   =  newCompareValue;

   current_value=lptmr_get_current_value();
   if (delayTicks<current_value-temp_last_compare_value) {
      // we're already too late, schedule the ISR right now manually

      // setting the interrupt flag triggers an interrupt
      // TODO .. look how.
	   bsp_timer_isr();
   } else {
      // this is the normal case, have timer expire at newCompareValue
	  lptmr_set_compare(newCompareValue);
	  lptmr_enable(); //in case not enabled
   }
}

/**
\brief Cancel a running compare.
*/
void bsp_timer_cancel_schedule() {
	lptmr_reset_compare();
	lptmr_disable();
}


PORT_TIMER_WIDTH bsp_timer_get_currentValue(){
	return lptmr_get_current_value();
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

uint8_t bsp_timer_isr() {
   // call the callback
   bsp_timer_vars.cb();
   // kick the OS
   return 1;
}

