/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:59.139127.
*/
#ifndef __UDPLATENCY_H
#define __UDPLATENCY_H

/**
\addtogroup AppUdp
\{
\addtogroup UdpLatency
\{
*/

//=========================== define ==========================================

/// inter-packet period (in mseconds)
#define UDPLATENCYPERIOD     3000
#define NUMPKTTEST           300

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void udplatency_init(OpenMote* self);
void udplatency_trigger(void);
void udplatency_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void udplatency_receive(OpenMote* self, OpenQueueEntry_t* msg);
bool udplatency_debugPrint(void);
void udplatency_task(OpenMote* self);

/**
\}
\}
*/

#endif
