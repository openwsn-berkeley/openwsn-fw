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
    sctimer_cbt      sctimer_action_cb;
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
    
    PORT_TIMER_WIDTH currentTime;

    currentTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__COUNTER);
    
    sctimer_enable();
    if (currentTime - val < TIMERLOOP_THRESHOLD){
        // the timer is already late, schedule the ISR right now manually 
        // not sure how to do this in scum, just miss this ISR for now
        // don't clear the flag, so another interrupt will happen after exiting the ISR
        sctimer_vars.noNeedClearFlag = 1;
    } else {
        if (val-currentTime<MINIMUM_COMPAREVALE_ADVANCE){
            // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
            // schedule ISR right now manually
            // don't clear the flag, so another interrupt will happen after exiting the ISR
            sctimer_vars.noNeedClearFlag = 1;
        } else {
            // mark clear the flag here
            // schedule the timer at val
            RFTIMER_REG__COMPARE0           = (PORT_TIMER_WIDTH)(TIMER_COUNTER_CONVERT_32K_TO_RFTIMER_CLK(val) & RFTIMER_MAX_COUNT);
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
    return TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__COUNTER);
}

void sctimer_enable(void){
    // enable compare interrupt (this also cancels any pending interrupts)
    RFTIMER_REG__COMPARE0_CONTROL   = RFTIMER_COMPARE_ENABLE |   \
                                      RFTIMER_COMPARE_INTERRUPT_ENABLE;
}

void sctimer_disable(void){
    RFTIMER_REG__COMPARE0_CONTROL = 0x0;
}

#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT

void sctimer_set_actionCallback(sctimer_cbt cb){
    sctimer_vars.sctimer_action_cb = cb;
}

void sctimer_scheduleActionIn(uint8_t type,PORT_RADIOTIMER_WIDTH offset){
    switch(type){
        case ACTION_LOAD_PACKET:
            // offset when to fire
            RFTIMER_REG__COMPARE3           = TIMER_COUNTER_CONVERT_32K_TO_RFTIMER_CLK(offset);
            
            // enable compare and tx load interrupt (this also cancels any pending interrupts)
            RFTIMER_REG__COMPARE3_CONTROL   = RFTIMER_COMPARE_ENABLE           |\
                                              RFTIMER_COMPARE_INTERRUPT_ENABLE |\
                                              RFTIMER_COMPARE_TX_LOAD_ENABLE;
            break;
        case ACTION_SEND_PACKET:
            // offset when to fire
            RFTIMER_REG__COMPARE4           = TIMER_COUNTER_CONVERT_32K_TO_RFTIMER_CLK(offset);
            
            // enable compare and tx send interrupt (this also cancels any pending interrupts)
            RFTIMER_REG__COMPARE4_CONTROL   = RFTIMER_COMPARE_ENABLE           |\
                                              RFTIMER_COMPARE_INTERRUPT_ENABLE |\
                                              RFTIMER_COMPARE_TX_SEND_ENABLE;
            break;
        case ACTION_RADIORX_ENABLE:
            // offset when to fire
            RFTIMER_REG__COMPARE5           = TIMER_COUNTER_CONVERT_32K_TO_RFTIMER_CLK(offset);
            
            // enable compare and rx start interrupt (this also cancels any pending interrupts)
            RFTIMER_REG__COMPARE5_CONTROL   = RFTIMER_COMPARE_ENABLE           |\
                                              RFTIMER_COMPARE_INTERRUPT_ENABLE |\
                                              RFTIMER_COMPARE_RX_START_ENABLE;
            break;
        case ACTION_SET_TIMEOUT:
            // offset when to fire
            RFTIMER_REG__COMPARE2           = TIMER_COUNTER_CONVERT_32K_TO_RFTIMER_CLK(offset);
            
            // enable compare interrupt (this also cancels any pending interrupts)
            RFTIMER_REG__COMPARE2_CONTROL   = RFTIMER_COMPARE_ENABLE           |\
                                              RFTIMER_COMPARE_INTERRUPT_ENABLE;
            break;
        default:
            // should never happens
            break;
    }
}

void sctimer_actionCancel(uint8_t type){
    
    switch(type){
        case ACTION_LOAD_PACKET:
            // disable compare and tx load interrupt
            RFTIMER_REG__COMPARE3_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE3           = 0x00;
            break;
        case ACTION_SEND_PACKET:
            // disable compare and tx send interrupt
            RFTIMER_REG__COMPARE4_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE4           = 0x00;
            break;
        case ACTION_RADIORX_ENABLE:
            // disable compare and rx start interrupt
            RFTIMER_REG__COMPARE5_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE5           = 0x00;
            break;
        case ACTION_SET_TIMEOUT:
            // disable compare interrupt
            RFTIMER_REG__COMPARE2_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE2           = 0x00;
            break;
        case ACTION_TX_SFD_DONE:
            // disable tx SFD done capture interrupt
            RFTIMER_REG__CAPTURE0_CONTROL   = 0x00;
            break;
        case ACTION_RX_SFD_DONE:
            // disable rx SFD done capture interrupt
            RFTIMER_REG__CAPTURE1_CONTROL   = 0x00;
            break;
        case ACTION_TX_SEND_DONE:
            // disable tx SEND done capture interrupt
            RFTIMER_REG__CAPTURE2_CONTROL   = 0x00;
            break;
        case ACTION_RX_DONE:
            // disable rx SEND done capture interrupt
            RFTIMER_REG__CAPTURE3_CONTROL   = 0x00;
            break;
        case ACTION_ALL_RADIOTIMER_INTERRUPT:
            RFTIMER_REG__CAPTURE0_CONTROL   = 0x00;
            RFTIMER_REG__CAPTURE1_CONTROL   = 0x00;
            RFTIMER_REG__CAPTURE2_CONTROL   = 0x00;
            RFTIMER_REG__CAPTURE3_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE2_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE3_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE4_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE5_CONTROL   = 0x00;
            RFTIMER_REG__COMPARE2           = 0x00;
            RFTIMER_REG__COMPARE3           = 0x00;
            RFTIMER_REG__COMPARE4           = 0x00;
            RFTIMER_REG__COMPARE5           = 0x00;
            break;
        default:
            // should never happens
            break;
    }
}

void sctimer_setCapture(uint8_t type){
    switch(type){
        case ACTION_TX_SFD_DONE:
            // capture tx SFD done event
            RFTIMER_REG__CAPTURE0_CONTROL   = RFTIMER_CAPTURE_INTERRUPT_ENABLE |    \
                                              RFTIMER_CAPTURE_INPUT_SEL_TX_SFD_DONE;
            break;
        case ACTION_RX_SFD_DONE:
            // capture rx SFD done event
            RFTIMER_REG__CAPTURE1_CONTROL   = RFTIMER_CAPTURE_INTERRUPT_ENABLE |    \
                                              RFTIMER_CAPTURE_INPUT_SEL_RX_SFD_DONE;
            break;
        case ACTION_TX_SEND_DONE:
            // capture tx SEND done event
            RFTIMER_REG__CAPTURE2_CONTROL   = RFTIMER_CAPTURE_INTERRUPT_ENABLE |    \
                                              RFTIMER_CAPTURE_INPUT_SEL_TX_SEND_DONE;
            break;
        case ACTION_RX_DONE:
            // capture rx SEND done event
            RFTIMER_REG__CAPTURE3_CONTROL   = RFTIMER_CAPTURE_INTERRUPT_ENABLE |    \
                                              RFTIMER_CAPTURE_INPUT_SEL_RX_DONE;
            break;
        default:
            // should never happens
            break;
    }
}
#endif

// ========================== private =========================================

kick_scheduler_t sctimer_isr(void){
    
    PORT_RADIOTIMER_WIDTH interrupt_flag;
    
    debugpins_isr_set();
    
    interrupt_flag = RFTIMER_REG__INT;
    
    if (
        interrupt_flag & RFTIMER_REG__INT_COMPARE2_INT  ||
        interrupt_flag & RFTIMER_REG__INT_COMPARE3_INT  ||
        interrupt_flag & RFTIMER_REG__INT_COMPARE4_INT  ||
        interrupt_flag & RFTIMER_REG__INT_COMPARE5_INT
    ) {
        // Compare interrupt for scheduled timer
        if (sctimer_vars.sctimer_action_cb!=NULL) {
            // clear the responding interrupt bits
            if (interrupt_flag & RFTIMER_REG__INT_COMPARE2_INT) {
                RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE2_INT;
            }else {
                if (interrupt_flag & RFTIMER_REG__INT_COMPARE3_INT) {
                    RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE3_INT;
                } else {
                    if (interrupt_flag & RFTIMER_REG__INT_COMPARE4_INT){
                        RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE4_INT;
                    } else {
                        if (interrupt_flag & RFTIMER_REG__INT_COMPARE5_INT){
                            RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE5_INT;
                        }
                    }
                }
            }
            // call the callback
            sctimer_vars.sctimer_action_cb();
            debugpins_isr_clr();
            // kick the OS
            return KICK_SCHEDULER;
        }
    } else {
        // Compare interrupt for bsp timer
        if (interrupt_flag & RFTIMER_REG__INT_COMPARE0_INT) {
            if (sctimer_vars.sctimer_cb!=NULL) {
                sctimer_vars.noNeedClearFlag = 0;
                sctimer_vars.sctimer_cb();
                if (sctimer_vars.noNeedClearFlag==0){
                    RFTIMER_REG__INT_CLEAR = RFTIMER_REG__INT_COMPARE0_INT;
                }
                debugpins_isr_clr();
                return KICK_SCHEDULER;
            }
        } else {
            if (interrupt_flag & RFTIMER_REG__INT_CAPTURE0_INT){
                RFTIMER_REG__INT_CLEAR = RFTIMER_REG__INT_CAPTURE0_INT;
            } else {
                if (interrupt_flag & RFTIMER_REG__INT_CAPTURE1_INT) {
                    RFTIMER_REG__INT_CLEAR = RFTIMER_REG__INT_CAPTURE1_INT;
                } else {
                    if (interrupt_flag & RFTIMER_REG__INT_CAPTURE2_INT) {
                        RFTIMER_REG__INT_CLEAR = RFTIMER_REG__INT_CAPTURE2_INT;
                    } else {
                        if (interrupt_flag & RFTIMER_REG__INT_CAPTURE3_INT) {
                            RFTIMER_REG__INT_CLEAR = RFTIMER_REG__INT_CAPTURE3_INT;
                        } else {
                            // this should not happen
                            RFTIMER_REG__INT_CLEAR      = interrupt_flag;
                        }
                    }
                }
            }
            debugpins_isr_clr();
            return KICK_SCHEDULER;
        }
    }
    debugpins_isr_clr();
    RFTIMER_REG__INT_CLEAR = RFTIMER_REG__INT_COMPARE0_INT;
    return DO_NOT_KICK_SCHEDULER;
}