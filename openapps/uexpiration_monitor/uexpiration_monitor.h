#ifndef __UMONITOR_H
#define __UMONITOR_H

/**
\addtogroup AppUdp
\{
\addtogroup umonitor
\{
*/

#include "config.h"
#include "udp.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================


//=========================== prototypes ======================================

void umonitor_init(void);

void umonitor_receive(OpenQueueEntry_t *msg);

void umonitor_sendDone(OpenQueueEntry_t *msg, owerror_t error);

bool umonitor_debugPrint(void);

/**
\}
\}
*/

#endif
