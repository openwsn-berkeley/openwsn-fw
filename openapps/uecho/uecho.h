#ifndef __UECHO_H
#define __UECHO_H

/**
\addtogroup AppUdp
\{
\addtogroup uecho
\{
*/

#include "config.h"
#include "openudp.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   udp_resource_desc_t desc;  ///< resource descriptor for this module, used to register at UDP stack
} uecho_vars_t;

//=========================== prototypes ======================================

void uecho_init(void);
void uecho_receive(OpenQueueEntry_t* msg);
void uecho_sendDone(OpenQueueEntry_t* msg, owerror_t error);
bool uecho_debugPrint(void);

/**
\}
\}
*/

#endif
