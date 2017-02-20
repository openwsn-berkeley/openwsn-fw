#ifndef __UINFO_H
#define __UINFO_H

/**
\addtogroup AppUdp
\{
\addtogroup uinfo
\{
*/

#include "opentimers.h"
#include "stats.h"


//=========================== define ==========================================

#define UINFO_PERIOD_MS 60000

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   opentimer_id_t       timerId;  ///< periodic timer which triggers transmission
   uint16_t             counter;  ///< incrementing counter which is written into the packet
   uint16_t              period;  ///< uinfo packet sending period>
} uinfo_vars_t;

//=========================== prototypes ======================================

void uinfo_init(void);
void uinfo_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void uinfo_receive(OpenQueueEntry_t* msg);
/**
\}
\}
*/

#endif
