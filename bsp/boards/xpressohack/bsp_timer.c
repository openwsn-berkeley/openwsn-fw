/**
\brief LPC-specific definition of the "timers" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/

#include "string.h"
#include "timer.h"
#include "bsp_timer.h"
#include "board.h"

typedef struct {
   bsp_timer_cbt    cb;
   PORT_TIMER_WIDTH last_compare_value;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================
void timer_compare_isr_2(uint8_t reg);
void timer_capture_isr_2(uint8_t reg);

//=========================== public ==========================================

/**
\brief Initialize this module.

This functions starts the timer, i.e. the counter increments, but doesn't set
any compare registers, so no interrupt will fire.
*/
void bsp_timer_init() {

   // clear local variables
   memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));

   timer_set_isr_compare_hook(TIMER_NUM2,bsp_timer_isr);
   timer_set_isr_capture_hook(TIMER_NUM2,bsp_timer_isr);

   timer_init(TIMER_NUM2);
   timer_enable(TIMER_NUM2);

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

	timer_reset_compare(TIMER_NUM2,TIMER_COMPARE_REG0);
	timer_reset(TIMER_NUM2);

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
   bsp_timer_vars.last_compare_value   =  newCompareValue;

   current_value=timer_get_current_value(TIMER_NUM2);
   if (delayTicks<current_value-temp_last_compare_value) {
      // we're already too late, schedule the ISR right now manually

      // setting the interrupt flag triggers an interrupt
      // TODO .. check that this works correctly.

	   timer_set_compare(TIMER_NUM2,TIMER_COMPARE_REG0,delayTicks+current_value);
	   timer_enable(TIMER_NUM2); //in case not enabled
	   //
   } else {
      // this is the normal case, have timer expire at newCompareValue
	  timer_set_compare(TIMER_NUM2,TIMER_COMPARE_REG0,newCompareValue);
	  timer_enable(TIMER_NUM2); //in case not enabled
   }
}

/**
\brief Cancel a running compare.
*/
void bsp_timer_cancel_schedule() {
	timer_reset_compare(TIMER_NUM2,TIMER_COMPARE_REG0);
	timer_disable(TIMER_NUM2);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

uint8_t bsp_timer_isr() {
   // call the callback
   bsp_timer_vars.cb();
   // kick the OS
   return 1;
}

void timer_compare_isr_2(uint8_t reg){
	if (reg==TIMER_COMPARE_REG0){
		bsp_timer_isr();
	}else{
		//error!
		while(1);
	}
}

void timer_capture_isr_2(uint8_t reg){
	//do nothing
}
