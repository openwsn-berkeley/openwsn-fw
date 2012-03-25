/**
\brief Cross-platform declaration "bsp_timer" bsp module.

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

void               bsp_timer_init();
void               bsp_timer_set_callback(bsp_timer_cbt cb);
void               bsp_timer_set_compare(uint16_t compareValue);
void               bsp_timer_cancel_compare();
PORT_TIMER_WIDTH   bsp_timer_get_current_value();

uint8_t bsp_timer_isr();

#endif
