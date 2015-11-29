#ifndef __WATCHDOG_H
#define __WATCHDOG_H

/**
\addtogroup cross-layers
\{
\addtogroup watchdog
\{
*/

#include "opendefs.h"
#include "opentimers.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   opentimer_id_t watchdogTimerId;
   uint8_t bones;
} watchdog_vars_t;

//=========================== prototypes ======================================

void     watchdog_init(void);

/**
\}
\}
*/

#endif
