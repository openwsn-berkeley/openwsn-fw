/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:11.660374.
*/
#ifndef __UDPINJECT_H
#define __UDPINJECT_H

/**
\addtogroup AppUdp
\{
\addtogroup udpInject
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void udpinject_init(OpenMote* self);
void udpinject_trigger(OpenMote* self);
void udpinject_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void udpinject_receive(OpenMote* self, OpenQueueEntry_t* msg);
bool udpinject_debugPrint(OpenMote* self);

/**
\}
\}
*/

#endif
