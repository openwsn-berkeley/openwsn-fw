/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:11.693200.
*/
#ifndef __UDPPRINT_H
#define __UDPPRINT_H

/**
\addtogroup AppUdp
\{
\addtogroup udpPrint
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void udpprint_init(OpenMote* self);
void udpprint_receive(OpenMote* self, OpenQueueEntry_t* msg);
void udpprint_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
bool udpprint_debugPrint(OpenMote* self);

/**
\}
\}
*/

#endif
