/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:59.631314.
*/
#ifndef __UDPRAND_H
#define __UDPRAND_H

/**
\addtogroup AppUdp
\{
\addtogroup UdpRand
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void udprand_init(OpenMote* self);
void udprand_trigger(void);
void udprand_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void udprand_receive(OpenMote* self, OpenQueueEntry_t* msg);
bool udprand_debugPrint(void);
void udprand_task(OpenMote* self);

/**
\}
\}
*/

#endif
