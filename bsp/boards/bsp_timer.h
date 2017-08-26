#ifndef __BSP_TIMERS_H
#define __BSP_TIMERS_H

/**
\addtogroup BSP
\{
\addtogroup bsp_timer
\{

\brief Cross-platform declaration "bsp_timer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "stdint.h"
#include "board.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*bsp_timer_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

void               bsp_timer_init(void);
void               bsp_timer_set_callback(bsp_timer_cbt cb);
void               bsp_timer_reset(void);
void               bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks);
void               bsp_timer_cancel_schedule(void);
PORT_TIMER_WIDTH   bsp_timer_get_currentValue(void);

// interrupt handlers
kick_scheduler_t   bsp_timer_isr(void);

/**
\}
\}
*/

#endif
