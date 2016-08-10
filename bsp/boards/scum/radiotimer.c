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

//=========================== define =======================================

// ==== radio timer control bit

#define RADIOTIMER_ENABLE                       0x01
#define RADIOTIMER_INTERRUPT_ENABLE             0x02
#define RADIOTIMER_COUNT_RESET                  0x04

// ==== radio timer interruption flag

#define RADIOTIMER_COMPARE0_INT                 0x0001
#define RADIOTIMER_COMPARE1_INT                 0x0002
#define RADIOTIMER_COMPARE2_INT                 0x0004
#define RADIOTIMER_COMPARE3_INT                 0x0008
#define RADIOTIMER_COMPARE4_INT                 0x0010
#define RADIOTIMER_COMPARE5_INT                 0x0020
#define RADIOTIMER_COMPARE6_INT                 0x0040
#define RADIOTIMER_COMPARE7_INT                 0x0080
#define RADIOTIMER_CAPTURE0_INT                 0x0100
#define RADIOTIMER_CAPTURE1_INT                 0x0200
#define RADIOTIMER_CAPTURE2_INT                 0x0400
#define RADIOTIMER_CAPTURE3_INT                 0x0800
#define RADIOTIMER_CAPTURE0_OVERFLOW_INT        0x1000
#define RADIOTIMER_CAPTURE1_OVERFLOW_INT        0x2000
#define RADIOTIMER_CAPTURE2_OVERFLOW_INT        0x4000
#define RADIOTIMER_CAPTURE3_OVERFLOW_INT        0x8000

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
    while(1);
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
    while(1);
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
    
    // set period of radiotimer
    RFTIMER_REG__MAX_COUNT          = TIMER_COUTER_CONVERT_32K_TO_500K(period);
    // enable timer and interrupt
    RFTIMER_REG__CONTROL            = RADIOTIMER_ENABLE         |   \
                                      RADIOTIMER_INTERRUPT_ENABLE;
    
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

//===== capture

PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
    return TIMER_COUTER_CONVERT_500K_TO_32K(RFTIMER_REG__COUNTER);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radiotimer_isr() {
    
    PORT_RADIOTIMER_WIDTH counter        = RFTIMER_REG__COUNTER;
    PORT_RADIOTIMER_WIDTH interrupt_flag = RFTIMER_REG__INT;
    if (interrupt_flag & RADIOTIMER_COMPARE2_INT) {
            // timer compare interrupt
            if (radiotimer_vars.compare_cb!=NULL) {
                // call the callback
                radiotimer_vars.compare_cb();
                RFTIMER_REG__INT_CLEAR  = RADIOTIMER_COMPARE2_INT;
                // kick the OS
                return KICK_SCHEDULER;
            }
    } else {
        if (interrupt_flag & RADIOTIMER_COMPARE1_INT) {
            // timer overflows interrupt
            if (radiotimer_vars.overflow_cb!=NULL) {
                // call the callback
                radiotimer_vars.overflow_cb();
                RFTIMER_REG__INT_CLEAR  = RADIOTIMER_COMPARE1_INT;
                // kick the OS
                return KICK_SCHEDULER;
            }
        } else {
            if (interrupt_flag & RADIOTIMER_COMPARE0_INT) {
                // bsp timer interrupt is handled in radiotimer module
                // call the callback
                bsp_timer_isr();
                RFTIMER_REG__INT_CLEAR      = RADIOTIMER_COMPARE0_INT;
                // kick the OS
                return KICK_SCHEDULER;
            } else {
                RFTIMER_REG__INT_CLEAR      = interrupt_flag;
                while(1);   // this should not happen
            }
        }
    }
    RFTIMER_REG__INT_CLEAR      = interrupt_flag;
    return DO_NOT_KICK_SCHEDULER;
}