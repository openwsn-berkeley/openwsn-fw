#ifndef OPENWSN_OPENQUEUE_H
#define OPENWSN_OPENQUEUE_H

/**
\addtogroup cross-layers
\{
\addtogroup OpenQueue
\{
*/

#include "config.h"
#include "opendefs.h"
#include "IEEE802154.h"

//=========================== define ==========================================

#define QUEUELENGTH  20

#if defined(OPENWSN_6LO_FRAGMENTATION_C)
#define BIGQUEUELENGTH  OPENWSN_MAX_NUM_BIGPKTS
#else
#define BIGQUEUELENGTH  0
#endif

//=========================== typedef =========================================

typedef struct {
    uint8_t creator;
    uint8_t owner;
} debugOpenQueueEntry_t;

//=========================== module variables ================================

typedef struct {
    OpenQueueEntry_t queue[QUEUELENGTH];
#if defined(OPENWSN_6LO_FRAGMENTATION_C)
    OpenQueueBigEntry_t big_queue[BIGQUEUELENGTH];
#endif
} openqueue_vars_t;

//=========================== prototypes ======================================

// admin
void openqueue_init(void);

bool debugPrint_queue(void);

// called by any component
OpenQueueEntry_t* openqueue_getFreePacketBuffer(uint8_t creator);

#if defined(OPENWSN_6LO_FRAGMENTATION_C)
OpenQueueEntry_t*  openqueue_getFreeBigPacketBuffer(uint8_t creator);
#endif

owerror_t openqueue_freePacketBuffer(OpenQueueEntry_t *pkt);

void openqueue_removeAllCreatedBy(uint8_t creator);

bool openqueue_isHighPriorityEntryEnough(void);

// called by ICMPv6
void openqueue_updateNextHopPayload(open_addr_t *newNextHop);

// called by res
OpenQueueEntry_t* openqueue_sixtopGetSentPacket(void);

OpenQueueEntry_t* openqueue_sixtopGetReceivedPacket(void);

uint8_t openqueue_getNum6PResp(void);

uint8_t openqueue_getNum6PReq(open_addr_t *neighbor);

void openqueue_remove6PrequestToNeighbor(open_addr_t *neighbor);

// called by IEEE80215E
OpenQueueEntry_t* openqueue_macGetEBPacket(void);

OpenQueueEntry_t* openqueue_macGetKaPacket(open_addr_t *toNeighbor);

OpenQueueEntry_t* openqueue_macGetDIOPacket(void);

OpenQueueEntry_t* openqueue_macGetUnicastPakcet(open_addr_t *toNeighbor);
/**
\}
\}
*/

#endif /* OPENWSN_OPENQUEUE_H */
