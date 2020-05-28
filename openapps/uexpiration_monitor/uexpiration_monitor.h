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

typedef struct {
   udp_resource_desc_t desc;  ///< resource descriptor for this module, used to register at UDP stack
} umonitor_vars_t;

//=========================== prototypes ======================================

void umonitor_init(void);
void umonitor_receive(OpenQueueEntry_t* msg);
void umonitor_sendDone(OpenQueueEntry_t* msg, owerror_t error);
bool umonitor_debugPrint(void);

/**
\}
\}
*/

#endif
