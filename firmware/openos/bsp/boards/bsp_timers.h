/**
\brief Cross-platform declaration "bsp_timers" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#ifndef __BSP_TIMERS_H
#define __BSP_TIMERS_H

#include "stdint.h"
#include "board_info.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*bsp_timer_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

void               bsp_timers_init();
void               bsp_timers_set_callback(bsp_timer_cbt cb);
void               bsp_timers_set_compare(uint16_t compareValue);
void               bsp_timers_cancel_compare();
PORT_TIMER_WIDTH   bsp_timers_get_current_value();

uint8_t bsp_timer_isr();

#endif
