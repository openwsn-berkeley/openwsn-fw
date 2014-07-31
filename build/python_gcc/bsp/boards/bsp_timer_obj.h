/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:59.149352.
*/
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
#include "board_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*bsp_timer_cbt)(OpenMote* self);

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void bsp_timer_init(OpenMote* self);
void bsp_timer_set_callback(OpenMote* self, bsp_timer_cbt cb);
void bsp_timer_reset(OpenMote* self);
void bsp_timer_scheduleIn(OpenMote* self, PORT_TIMER_WIDTH delayTicks);
void bsp_timer_cancel_schedule(OpenMote* self);
PORT_TIMER_WIDTH bsp_timer_get_currentValue(OpenMote* self);

// interrupt handlers
kick_scheduler_t bsp_timer_isr(OpenMote* self);

/**
\}
\}
*/

#endif
