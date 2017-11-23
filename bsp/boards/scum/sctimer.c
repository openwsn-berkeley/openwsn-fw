/**
\brief SCuM-specific definition of the "sctimer" bsp module, A timer module with only a single compare value. 

\author Tengfei Chang     <tengfei.chang@inria.fr> Novemeber 22, 2017.
*/

#include "memory_map.h"
#include "string.h"
#include "sctimer.h"
#include "board.h"
#include "debugpins.h"

// ========================== define ==========================================

#define RFTIMER_MAX_COUNT            0x3ffffff       // use a value less than 0xffffffff/61 and also equal to 2^n-1
#define TIMERLOOP_THRESHOLD            0xfffff       // 0xffff is 2 seconds @ 32768Hz clock
#define MINIMUM_COMPAREVALE_ADVANCE  10

// ========================== variable ========================================

typedef struct {
    sctimer_cbt      sctimer_cb;
    PORT_TIMER_WIDTH last_compare_value;
    uint8_t          noNeedClearFlag;
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
    
    // set period of radiotimer
    RFTIMER_REG__MAX_COUNT          = RFTIMER_MAX_COUNT;
    // enable timer and interrupt
    RFTIMER_REG__CONTROL            = 0x07;
}

void sctimer_set_callback(sctimer_cbt cb){
    sctimer_vars.sctimer_cb = cb;
}

/**
\brief set compare interrupt
*/
void sctimer_setCompare(PORT_TIMER_WIDTH val){
    sctimer_enable();
    if (TIMER_COUNTER_CONVERT_500K_TO_32K(RFTIMER_REG__COUNTER) - val < TIMERLOOP_THRESHOLD){
        // the timer is already late, schedule the ISR right now manually 
        // not sure how to do this in scum, just miss this ISR for now
        // don't clear the flag, so another interrupt will happen after exiting the ISR
        sctimer_vars.noNeedClearFlag = 1;
    } else {
        if (val-TIMER_COUNTER_CONVERT_500K_TO_32K(RFTIMER_REG__COUNTER)<MINIMUM_COMPAREVALE_ADVANCE){
            // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
            // schedule ISR right now manually
            // don't clear the flag, so another interrupt will happen after exiting the ISR
            sctimer_vars.noNeedClearFlag = 1;
        } else {
            // mark clear the flag here
            // schedule the timer at val
            RFTIMER_REG__COMPARE0           = (PORT_TIMER_WIDTH)(TIMER_COUNTER_CONVERT_32K_TO_500K(val) & RFTIMER_MAX_COUNT);
            RFTIMER_REG__COMPARE0_CONTROL   = RFTIMER_COMPARE_ENABLE |   \
                                              RFTIMER_COMPARE_INTERRUPT_ENABLE;
        }
    }
}

/**
\brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH sctimer_readCounter(void){
    return TIMER_COUNTER_CONVERT_500K_TO_32K(RFTIMER_REG__COUNTER);
}

void sctimer_enable(void){
    // enable compare interrupt (this also cancels any pending interrupts)
    RFTIMER_REG__COMPARE0_CONTROL   = RFTIMER_COMPARE_ENABLE |   \
                                      RFTIMER_COMPARE_INTERRUPT_ENABLE;
}

void sctimer_disable(void){
    RFTIMER_REG__COMPARE0_CONTROL = 0x0;
}

// ========================== private =========================================

kick_scheduler_t sctimer_isr(void){
    debugpins_isr_set();
    if (sctimer_vars.sctimer_cb!=NULL) {
        sctimer_vars.noNeedClearFlag = 0;
        sctimer_vars.sctimer_cb();
        debugpins_isr_clr();
        if (sctimer_vars.noNeedClearFlag==0){
            RFTIMER_REG__INT_CLEAR = RFTIMER_REG__INT_COMPARE0_INT;
        }
        return KICK_SCHEDULER;
    }
    debugpins_isr_clr();
    RFTIMER_REG__INT_CLEAR = RFTIMER_REG__INT_COMPARE0_INT;
    return DO_NOT_KICK_SCHEDULER;
}