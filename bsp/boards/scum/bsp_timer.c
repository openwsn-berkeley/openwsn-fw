/**
\brief SCuM-specific definition of the "bsp_timer" bsp module.

On SCuM, there is no bsp timer 0 currently. So the implementation 
is empty by now.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/

#include "memory_map.h"
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

This functions starts the timer, i.e. the counter increments, but doesn't set
any compare registers, so no interrupt will fire.
*/
void bsp_timer_init(void) {
   
    // clear local variables
    memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));
    
    // set period of radiotimer
    RFTIMER_REG__MAX_COUNT          = TIMER_COUTER_CONVERT_32K_TO_500K(PORT_TsSlotDuration);
    // enable timer and interrupt
    RFTIMER_REG__CONTROL            = 0x07;
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
void bsp_timer_reset(void) {
    // record last timer compare value
    bsp_timer_vars.last_compare_value   = 0;
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
    
    temp_last_compare_value             = bsp_timer_vars.last_compare_value;
    
    newCompareValue                     = bsp_timer_vars.last_compare_value+delayTicks+1;
    newCompareValue                    %= TIMER_COUTER_CONVERT_500K_TO_32K(RFTIMER_REG__MAX_COUNT);
    bsp_timer_vars.last_compare_value   = newCompareValue;
    
    if (delayTicks<TIMER_COUTER_CONVERT_500K_TO_32K(RFTIMER_REG__COUNTER)-temp_last_compare_value) {
        // we're already too late, schedule the ISR right now manually
        // not sure how to do this in scum, just miss this ISR for now

    } else {
        // this is the normal case, have timer expire at newCompareValue
        RFTIMER_REG__COMPARE0           = (PORT_TIMER_WIDTH)(TIMER_COUTER_CONVERT_32K_TO_500K(newCompareValue));
        RFTIMER_REG__COMPARE0_CONTROL   = RFTIMER_COMPARE_ENABLE |   \
                                          RFTIMER_COMPARE_INTERRUPT_ENABLE;
    }
}

/**
\brief Cancel a running compare.
*/
void bsp_timer_cancel_schedule(void) {
    RFTIMER_REG__COMPARE0_CONTROL   = 0x00;
}

/**
\brief Return the current value of the timer's counter.

\returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH bsp_timer_get_currentValue(void) {
    return TIMER_COUTER_CONVERT_500K_TO_32K(RFTIMER_REG__COUNTER);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t bsp_timer_isr(void) {
    
    // call the callback
    bsp_timer_vars.cb();
    
    // kick the OS
    return KICK_SCHEDULER;
}