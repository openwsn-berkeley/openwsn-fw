#ifndef __FRAGTEST_H
#define __FRAGTEST_H

/**
\addtogroup AppUdp
\{
\addtogroup fragtest
\{
*/

#include "opentimers.h"

//=========================== define ==========================================

#define FRAGTEST_PERIOD_MS 70000

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   opentimer_id_t       timerId;  ///< periodic timer which triggers transmission
   uint16_t             counter;  ///< incrementing counter which is written into the packet
} fragtest_vars_t;

//=========================== prototypes ======================================

void fragtest_init(void);
void fragtest_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void fragtest_receive(OpenQueueEntry_t* msg);

/**
\}
\}
*/

#endif
