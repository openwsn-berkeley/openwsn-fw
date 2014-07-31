/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:01:58.295670.
*/
#ifndef __OPENQUEUE_H
#define __OPENQUEUE_H

/**
\addtogroup cross-layers
\{
\addtogroup OpenQueue
\{
*/

#include "openwsn_obj.h"
#include "IEEE802154_obj.h"

//=========================== define ==========================================

#define QUEUELENGTH  10

//=========================== typedef =========================================

typedef struct {
   uint8_t  creator;
   uint8_t  owner;
} debugOpenQueueEntry_t;

//=========================== module variables ================================

typedef struct {
   OpenQueueEntry_t queue[QUEUELENGTH];
} openqueue_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

// admin
void openqueue_init(OpenMote* self);
bool debugPrint_queue(OpenMote* self);
// called by any component
OpenQueueEntry_t* openqueue_getFreePacketBuffer(OpenMote* self, uint8_t creator);
owerror_t openqueue_freePacketBuffer(OpenMote* self, OpenQueueEntry_t* pkt);
void openqueue_removeAllCreatedBy(OpenMote* self, uint8_t creator);
void openqueue_removeAllOwnedBy(OpenMote* self, uint8_t owner);
// called by res
OpenQueueEntry_t* openqueue_sixtopGetSentPacket(OpenMote* self);
OpenQueueEntry_t* openqueue_sixtopGetReceivedPacket(OpenMote* self);
// called by IEEE80215E
OpenQueueEntry_t* openqueue_macGetDataPacket(OpenMote* self, open_addr_t* toNeighbor);
OpenQueueEntry_t* openqueue_macGetAdvPacket(OpenMote* self);

/**
\}
\}
*/

#endif
