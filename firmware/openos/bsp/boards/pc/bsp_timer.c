/**
\brief PC-specific definition of the "bsp_timer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "bsp_timer.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   bsp_timer_cbt    cb;
   PORT_TIMER_WIDTH last_compare_value;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================

//=========================== callbacks =======================================

void bsp_timer_set_callback(bsp_timer_cbt cb) {
   bsp_timer_vars.cb   = cb;
}

//=========================== public ==========================================

void bsp_timer_init() {
   
   // clear local variables
   memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));
   
   // poipoipoi stub
   printf("TODO bsp_timer_init\r\n");
}

void bsp_timer_reset() {
   // poipoipoi stub
   printf("TODO bsp_timer_reset\r\n");
}

void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks) {
   // poipoipoi stub
   printf("TODO bsp_timer_scheduleIn\r\n");
}

void bsp_timer_cancel_schedule() {
   // poipoipoi stub
   printf("TODO bsp_timer_cancel_schedule\r\n");
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================