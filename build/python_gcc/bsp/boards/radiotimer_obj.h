/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:32.291996.
*/
#ifndef __RADIOTIMER_H
#define __RADIOTIMER_H

/**
\addtogroup BSP
\{
\addtogroup radiotimer
\{

\brief Cross-platform declaration "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
#include "board_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*radiotimer_compare_cbt)(OpenMote* self);
typedef void (*radiotimer_capture_cbt)(OpenMote* self, PORT_TIMER_WIDTH timestamp);

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

// admin
void radiotimer_init(OpenMote* self);
void radiotimer_setOverflowCb(OpenMote* self, radiotimer_compare_cbt cb);
void radiotimer_setCompareCb(OpenMote* self, radiotimer_compare_cbt cb);
void radiotimer_setStartFrameCb(OpenMote* self, radiotimer_capture_cbt cb);
void radiotimer_setEndFrameCb(OpenMote* self, radiotimer_capture_cbt cb);
void radiotimer_start(OpenMote* self, PORT_RADIOTIMER_WIDTH period);
// direct access
PORT_RADIOTIMER_WIDTH radiotimer_getValue(OpenMote* self);
void radiotimer_setPeriod(OpenMote* self, PORT_RADIOTIMER_WIDTH period);
PORT_RADIOTIMER_WIDTH radiotimer_getPeriod(OpenMote* self);
// compare
void radiotimer_schedule(OpenMote* self, PORT_RADIOTIMER_WIDTH offset);
void radiotimer_cancel(OpenMote* self);
// capture
PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(OpenMote* self);

// interrupt handlers
kick_scheduler_t radiotimer_isr(OpenMote* self);

/**
\}
\}
*/

#endif
