/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:06.029224.
*/
#ifndef __TCPINJECT_H
#define __TCPINJECT_H

/**
\addtogroup AppTcp
\{
\addtogroup tcpInject
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   OpenQueueEntry_t*    pkt;
   bool                 sending;
   open_addr_t          hisAddress;
   uint16_t             hisPort;
} tcpinject_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void tcpinject_init(OpenMote* self);
bool tcpinject_shouldIlisten(OpenMote* self);
void tcpinject_trigger(OpenMote* self);
void tcpinject_connectDone(OpenMote* self, owerror_t error);
void tcpinject_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void tcpinject_receive(OpenMote* self, OpenQueueEntry_t* msg);
bool tcpinject_debugPrint(OpenMote* self);

/**
\}
\}
*/

#endif
