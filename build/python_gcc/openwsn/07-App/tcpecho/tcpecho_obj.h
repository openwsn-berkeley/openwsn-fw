/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:10.657279.
*/
#ifndef __TCPECHO_H
#define __TCPECHO_H

/**
\addtogroup AppTcp
\{
\addtogroup tcpEcho
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void tcpecho_init(OpenMote* self);
bool tcpecho_shouldIlisten(OpenMote* self);
void tcpecho_receive(OpenMote* self, OpenQueueEntry_t* msg);
void tcpecho_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void tcpecho_connectDone(OpenMote* self, owerror_t error);
bool tcpecho_debugPrint(OpenMote* self);

/**
\}
\}
*/

#endif
