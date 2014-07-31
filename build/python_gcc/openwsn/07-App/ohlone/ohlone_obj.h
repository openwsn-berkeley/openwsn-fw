/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:01:57.489473.
*/
#ifndef __OHLONE_H
#define __OHLONE_H

/**
\addtogroup AppTcp
\{
\addtogroup ohlone
\{
*/

#include "opentcp_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   OpenQueueEntry_t*    pkt;
   bool                 sending;
   uint16_t             httpChunk;
   uint8_t              getRequest[TCP_DEFAULT_WINDOW_SIZE];
} ohlone_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void ohlone_init(OpenMote* self);
bool ohlone_shouldIlisten(OpenMote* self);
void ohlone_receive(OpenMote* self, OpenQueueEntry_t* msg);
void ohlone_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void ohlone_connectDone(OpenMote* self, owerror_t error);
bool ohlone_debugPrint(OpenMote* self);

/**
\}
\}
*/

#endif
