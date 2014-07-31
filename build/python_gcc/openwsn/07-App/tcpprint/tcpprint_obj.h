/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:11.157403.
*/
#ifndef __TCPPRINT_H
#define __TCPPRINT_H

/**
\addtogroup AppTcp
\{
\addtogroup tcpPrint
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void tcpprint_init(OpenMote* self);
bool tcpprint_shouldIlisten(OpenMote* self);
void tcpprint_receive(OpenMote* self, OpenQueueEntry_t* msg);
void tcpprint_connectDone(OpenMote* self, owerror_t error);
void tcpprint_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
bool tcpprint_debugPrint(OpenMote* self);

/**
\}
\}
*/

#endif
