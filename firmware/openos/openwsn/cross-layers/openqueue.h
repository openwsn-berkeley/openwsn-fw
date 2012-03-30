#ifndef __OPENQUEUE_H
#define __OPENQUEUE_H

/**
\addtogroup cross-layers
\{
\addtogroup OpenQueue
\{
*/

#include "openwsn.h"
#include "IEEE802154.h"

//=========================== define ==========================================

#define QUEUELENGTH  10

//=========================== typedef =========================================

typedef struct {
   uint8_t  creator;
   uint8_t  owner;
} debugOpenQueueEntry_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
          void               openqueue_init();
          bool               debugPrint_queue();
// called by any component
 OpenQueueEntry_t*  openqueue_getFreePacketBuffer();
 error_t            openqueue_freePacketBuffer(OpenQueueEntry_t* pkt);
 void               openqueue_removeAllOwnedBy(uint8_t owner);
// called by res
 OpenQueueEntry_t*  openqueue_resGetSentPacket();
 OpenQueueEntry_t*  openqueue_resGetReceivedPacket();
// called by IEEE80215E
 OpenQueueEntry_t*  openqueue_macGetDataPacket(open_addr_t* toNeighbor);
 OpenQueueEntry_t*  openqueue_macGetAdvPacket();

/**
\}
\}
*/

#endif
