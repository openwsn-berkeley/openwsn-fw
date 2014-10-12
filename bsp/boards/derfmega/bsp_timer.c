/**
\brief derfmega-specific definition of the "bsp_timer" bsp module.

On derfmega, we use the MAC timer + COMPARE1

\author Kevin Weekly June 2012.
*/

#include <avr/io.h>
#include "string.h"
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

This functions starts the timer
*/
void bsp_timer_init() {
   
   // clear local variables
   memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));
   
   PRR0 &= ~(1<<PRTIM2); // turn on timer 2 for crystal
   SCCR0 = (SCCR0 | 0b00110000) & 0b11111110; // enable symbol counter, 32KHz clock, absolute compare 1
   SCCR1 = 0; // no backoff slot counter
   ASSR |= (1<<AS2); // enable 32KHz crystal

   //set compare1 registers
   SCOCR1HH = SCOCR1HL = SCOCR1LH = SCOCR1LL = 0;
   
   // don't enable interrupts until first compare is set
   
   // wait for register writes
   while(SCSR & 0x01); 
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
   // disable interrupts
   SCIRQM &= 0xFE;
   
   // reset timer
   SCCNTHH = SCCNTHL = SCCNTLH = 0;
   SCCNTLL = 0;
   
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
   
   current_value = bsp_timer_get_currentValue();
   if (current_value > temp_last_compare_value && delayTicks<current_value-temp_last_compare_value) {
      // we're already too late, schedule the ISR right now manually
      bsp_timer_isr();
   } else {
      // this is the normal case, have timer expire at newCompareValue
      SCOCR1HH  =  (uint8_t)(newCompareValue>>24);
	  SCOCR1HL  =  (uint8_t)(newCompareValue>>16);
	  SCOCR1LH  =  (uint8_t)(newCompareValue>>8);
	  SCOCR1LL  =  (uint8_t)(newCompareValue);
	  // enable interrupts
	  SCIRQM |= 0x01;
   }
}

/**
\brief Cancel a running compare.
*/
void bsp_timer_cancel_schedule() {
	SCIRQM &= 0xFE;
}
/**
\brief Return the current value of the timer's counter.

\returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
   PORT_TIMER_WIDTH retval = SCCNTLL;
   retval |= (PORT_TIMER_WIDTH)SCCNTLH << 8;
   retval |= (PORT_TIMER_WIDTH)SCCNTHL << 16;
   retval |= (PORT_TIMER_WIDTH)SCCNTHH << 24;
   return retval;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

uint8_t bsp_timer_isr() {
   // call the callback
   bsp_timer_vars.cb();
   // kick the OS
   return 1;
}