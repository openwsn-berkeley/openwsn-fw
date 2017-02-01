/**
\brief SCuM-specific definition of the "radiotimer" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/

#include "memory_map.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "radiotimer.h"
#include "board.h"
#include "bsp_timer.h"
#include "debugpins.h"

//=========================== define =======================================

//=========================== variables =======================================

typedef struct {
    radiotimer_compare_cbt    overflow_cb;
    radiotimer_compare_cbt    compare_cb;
    PORT_RADIOTIMER_WIDTH     interrupt_flag;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
    // clear local variables
    memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
    radiotimer_vars.overflow_cb    = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
    radiotimer_vars.compare_cb     = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
    // should never be called
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
    // should never be called
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
    
    // set period of radiotimer
    RFTIMER_REG__MAX_COUNT          = TIMER_COUTER_CONVERT_32K_TO_500K(period);
    // enable timer and interrupt
    RFTIMER_REG__CONTROL            = RFTIMER_REG__CONTROL_ENABLE         |   \
                                      RFTIMER_REG__CONTROL_INTERRUPT_ENABLE;
    
    // set compare timer counter 0 to perform an overflow interrupt
    RFTIMER_REG__COMPARE1           = 0;
    // enable compare0 module and interrup
    RFTIMER_REG__COMPARE1_CONTROL   = RFTIMER_COMPARE_ENABLE |   \
                                      RFTIMER_COMPARE_INTERRUPT_ENABLE;
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
    return TIMER_COUTER_CONVERT_500K_TO_32K(RFTIMER_REG__COUNTER);
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
    RFTIMER_REG__MAX_COUNT          = TIMER_COUTER_CONVERT_32K_TO_500K(period);
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
    return TIMER_COUTER_CONVERT_500K_TO_32K(RFTIMER_REG__MAX_COUNT);
}

//===== compare
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
void radiotimer_schedule(uint8_t type,PORT_RADIOTIMER_WIDTH offset) {
    switch(type){
        case ACTION_LOAD_PACKET:
            // offset when to fire
            RFTIMER_REG__COMPARE3           = TIMER_COUTER_CONVERT_32K_TO_500K(offset);
            
            // enable compare and tx load interrupt (this also cancels any pending interrupts)
            RFTIMER_REG__COMPARE3_CONTROL   = RFTIMER_COMPARE_ENABLE |          \
                                              RFTIMER_COMPARE_INTERRUPT_ENABLE |\
                                              RFTIMER_COMPARE_TX_LOAD_ENABLE;
            break;
        case ACTION_SEND_PACKET:
            // offset when to fire
            RFTIMER_REG__COMPARE4           = TIMER_COUTER_CONVERT_32K_TO_500K(offset);
            
            // enable compare and tx send interrupt (this also cancels any pending interrupts)
            RFTIMER_REG__COMPARE4_CONTROL   = RFTIMER_COMPARE_ENABLE |          \
                                              RFTIMER_COMPARE_TX_SEND_ENABLE;
            break;
        case ACTION_RADIORX_ENABLE:
            // offset when to fire
            RFTIMER_REG__COMPARE5           = TIMER_COUTER_CONVERT_32K_TO_500K(offset);
            
            // enable compare and rx start interrupt (this also cancels any pending interrupts)
            RFTIMER_REG__COMPARE5_CONTROL   = RFTIMER_COMPARE_ENABLE |          \
                                              RFTIMER_COMPARE_INTERRUPT_ENABLE |\
                                              RFTIMER_COMPARE_RX_START_ENABLE;
            break;
        case ACTION_NORMAL_TIMER:
            // offset when to fire
            RFTIMER_REG__COMPARE2           = TIMER_COUTER_CONVERT_32K_TO_500K(offset);
            
            // enable compare interrupt (this also cancels any pending interrupts)
            RFTIMER_REG__COMPARE2_CONTROL   = RFTIMER_COMPARE_ENABLE |          \
                                              RFTIMER_COMPARE_INTERRUPT_ENABLE;
            break;
        default:
            // should never happens
            break;
    }
}

void radiotimer_cancel(uint8_t type) {
    
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
        case ACTION_NORMAL_TIMER:
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

void radiotimer_setCapture(uint8_t type){
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
#else

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
    // offset when to fire
    RFTIMER_REG__COMPARE2           = TIMER_COUTER_CONVERT_32K_TO_500K(offset);
   
    // enable compare interrupt (this also cancels any pending interrupts)
    RFTIMER_REG__COMPARE2_CONTROL   = RFTIMER_COMPARE_ENABLE |   \
                                      RFTIMER_COMPARE_INTERRUPT_ENABLE;
}

void radiotimer_cancel() {
    // disable compare interrupt
    RFTIMER_REG__COMPARE2_CONTROL = 0x00;
}
#endif

//===== capture

PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
    return TIMER_COUTER_CONVERT_500K_TO_32K(RFTIMER_REG__COUNTER);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radiotimer_isr() {
    PORT_RADIOTIMER_WIDTH interrupt_flag = RFTIMER_REG__INT;
    debugpins_isr_set();
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    if (
        interrupt_flag & RFTIMER_REG__INT_COMPARE2_INT  ||
        interrupt_flag & RFTIMER_REG__INT_COMPARE3_INT  ||
        interrupt_flag & RFTIMER_REG__INT_COMPARE5_INT
        
    ) {
        // Compare interrupt for scheduled timer
        if (radiotimer_vars.compare_cb!=NULL) {
            // clear the responding interrupt bits
            if (interrupt_flag & RFTIMER_REG__INT_COMPARE3_INT) {
                RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE3_INT;
            }else {
                if (interrupt_flag & RFTIMER_REG__INT_COMPARE5_INT) {
                    RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE5_INT;
                } else {
                    RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE2_INT;
                }
            }
            // call the callback
            radiotimer_vars.compare_cb();
            debugpins_isr_clr();
            // kick the OS
            return KICK_SCHEDULER;
        }
    } else {
        // Compare interrupt for overflow timer ( fired at zero)
        if (interrupt_flag & RFTIMER_REG__INT_COMPARE1_INT) {
            // timer overflows interrupt
            if (radiotimer_vars.overflow_cb!=NULL) {
                // clear the interrupt bit and call the callback
                RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE1_INT;
                radiotimer_vars.overflow_cb();
                debugpins_isr_clr();
                // kick the OS
                return KICK_SCHEDULER;
            }
        } else {
            // Compare interrupt for bsp timer
            if (interrupt_flag & RFTIMER_REG__INT_COMPARE0_INT) {
                // bsp timer interrupt is handled in radiotimer module
                // clear the interrupt bit and call the callback
                RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE0_INT;
                bsp_timer_isr();
                debugpins_isr_clr();
                // kick the OS
                return KICK_SCHEDULER;
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
    }
#else
    if ( interrupt_flag & RFTIMER_REG__INT_COMPARE2_INT ) {
        // Compare interrupt for scheduled timer
        if (radiotimer_vars.compare_cb!=NULL) {
            // call the callback
            RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE2_INT;
            radiotimer_vars.compare_cb();
            debugpins_isr_clr();
            // kick the OS
            return KICK_SCHEDULER;
        }
    } else {
        // Compare interrupt for overflow timer ( fired at zero)
        if (interrupt_flag & RFTIMER_REG__INT_COMPARE1_INT) {
            // timer overflows interrupt
            if (radiotimer_vars.overflow_cb!=NULL) {
                // clear the interrupt bit and call the callback
                RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE1_INT;
                radiotimer_vars.overflow_cb();
                debugpins_isr_clr();
                // kick the OS
                return KICK_SCHEDULER;
            }
        } else {
            // Compare interrupt for bsp timer
            if (interrupt_flag & RFTIMER_REG__INT_COMPARE0_INT) {
                // bsp timer interrupt is handled in radiotimer module
                // clear the interrupt bit and call the callback
                RFTIMER_REG__INT_CLEAR  = RFTIMER_REG__INT_COMPARE0_INT;
                bsp_timer_isr();
                debugpins_isr_clr();
                // kick the OS
                return KICK_SCHEDULER;
            } else {
                
            }
        }
    }
#endif
    RFTIMER_REG__INT_CLEAR      = interrupt_flag;
    debugpins_isr_clr();
    return DO_NOT_KICK_SCHEDULER;
}