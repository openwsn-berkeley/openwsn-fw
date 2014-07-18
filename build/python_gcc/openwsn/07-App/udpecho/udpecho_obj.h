/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:11.252507.
*/
#ifndef __UDPECHO_H
#define __UDPECHO_H

/**
\addtogroup AppUdp
\{
\addtogroup udpEcho
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void udpecho_init(OpenMote* self);
void udpecho_receive(OpenMote* self, OpenQueueEntry_t* msg);
void udpecho_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
bool udpecho_debugPrint(OpenMote* self);

/**
\}
\}
*/

#endif
