/**
\brief Alternate version of the eZ430-RF2500 "bsp_timer" bsp module, which uses
       the lptimer module

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2012.
*/

//#include "io430.h"
#include "string.h"
#include "bsp_timer.h"
#include "board.h"
#include "board_info.h"
#include "lptmr.h"

//=========================== defines =========================================

//=========================== variables =======================================

/*
typedef struct {
   bsp_timer_cbt    cb;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;
*/

//=========================== prototypes ======================================

//=========================== public ==========================================

void bsp_timer_init() {
   lptimer_init();
}

void bsp_timer_set_callback(bsp_timer_cbt cb) {
   lptimer_bsp_timer_set_callback(cb);
}

void bsp_timer_reset() {
   lptimer_bsp_timer_reset();
}

void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks) {
   lptimer_bsp_timer_scheduleIn(delayTicks);
}

void bsp_timer_cancel_schedule() {
   lptimer_bsp_timer_cancel_schedule();
}

PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
   return lptimer_bsp_timer_get_currentValue();
}

//=========================== private =========================================