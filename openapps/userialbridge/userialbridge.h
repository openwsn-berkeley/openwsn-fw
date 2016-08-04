#ifndef __USERIALBRIDGE_H
#define __USERIALBRIDGE_H

/**
\addtogroup AppUdp
\{
\addtogroup userialbridge
\{
*/

#include "opentimers.h"

//=========================== define ==========================================

#define USERIALBRIDGE_PERIOD_MS 30000

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   opentimer_id_t       timerId;  ///< periodic timer which triggers transmission
   uint16_t             counter;  ///< incrementing counter which is written into the packet
   uint16_t              period;  ///< userialbridge packet sending period>
} userialbridge_vars_t;

//=========================== prototypes ======================================

void userialbridge_init(void);
void userialbridge_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void userialbridge_receive(OpenQueueEntry_t* msg);
/**
\}
\}
*/

#endif
