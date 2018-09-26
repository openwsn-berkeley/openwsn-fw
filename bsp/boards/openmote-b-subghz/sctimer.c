/**
\brief A timer module with only a single compare value. 

\author Tengfei Chang     <tengfei.chang@inria.fr> April 2017
*/

#include "board_info.h"
#include "sctimer.h"
#include "sleepmode.h"
#include "debugpins.h"
#include <headers/hw_ints.h>

// ========================== define ==========================================

#define TIMERLOOP_THRESHOLD          0xffffff     // 511 seconds @ 32768Hz clock
#define MINIMUM_COMPAREVALE_ADVANCE  10

// ========================== variable ========================================

typedef struct {
    sctimer_cbt sctimer_cb;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars;


// ========================== private =========================================

void sctimer_isr_internal(void);

// ========================== protocol =========================================

/**
\brief Initialization sctimer.
*/
void sctimer_init(void){
    memset(&sctimer_vars, 0, sizeof(sctimer_vars_t));
    IntRegister(INT_SMTIM, sctimer_isr_internal);
    IntDisable(INT_SMTIM);
}

void sctimer_set_callback(sctimer_cbt cb){
    sctimer_vars.sctimer_cb = cb;
}

/**
\brief set compare interrupt
*/
void sctimer_setCompare(uint32_t val){
    IntEnable(INT_SMTIM);
    // the current time is later than the required value, but close enough
    // to think we have been too slow to schedule it.
    if (SleepModeTimerCountGet() - val < TIMERLOOP_THRESHOLD){
        // the timer is already late, schedule the ISR right now manually 
        IntPendSet(INT_SMTIM);
    } else {
        if (val-SleepModeTimerCountGet()<MINIMUM_COMPAREVALE_ADVANCE){
            // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
            // schedule ISR right now manually
            IntPendSet(INT_SMTIM);
        } else {
            // schedule the timer at val
            SleepModeTimerCompareSet(val);
        }
    }
}

/**
\brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
*/
uint32_t sctimer_readCounter(void){
  return SleepModeTimerCountGet();
}

void sctimer_enable(void){
    IntEnable(INT_SMTIM);
}

void sctimer_disable(void){
    IntDisable(INT_SMTIM);
}

// ========================== private =========================================

void sctimer_isr_internal(void){
    debugpins_isr_set();
    if (sctimer_vars.sctimer_cb!=NULL) {
        IntPendClear(INT_SMTIM);
        sctimer_vars.sctimer_cb();
        debugpins_isr_clr();
    }
    debugpins_isr_clr();
}