#ifndef __OPENQUEUE_H
#define __OPENQUEUE_H

/**
\addtogroup cross-layers
\{
\addtogroup OpenQueue
\{
*/

#include "opendefs.h"
#include "IEEE802154.h"
#include "sixtop.h"


//=========================== define ==========================================

//Max number of packets in the queue
#define QUEUELENGTH           15
//DAO + DIO + 2 * SIXTOP
#define QUEUELENGTH_RESERVED  4
// By default, a packet should be removed after this timeout (we can consider safely it failed)
#define QUEUE_TIMEOUT_DEFAULT (SIX2SIX_TIMEOUT_MS * 5)
// Number of rows to send simultaneously to openserial
#define QUEUE_NBROWS_OPENSERIALSTATUS   1

//=========================== typedef =========================================

typedef struct {
   uint8_t     row;
   uint8_t     creator;
   uint8_t     owner;
   asn_t       timeout;    //timeout MUST be transmitted in ASN format to openvizualizer
   uint16_t    trackInstance;
   open_addr_t trackOwner;
   open_addr_t nextHop;
  // uint8_t     garbage;    //we MUST use an even nb. of bytes (else the compiler may have some uncompliant optimizations)
 } debugOpenQueueEntry_t;

//=========================== module variables ================================

typedef struct {
   OpenQueueEntry_t queue[QUEUELENGTH];
   uint8_t          debugPrintRow;     //the last row pushed to openVisualizer
   OpenQueueEntry_t openbridge;
} openqueue_vars_t;

//=========================== prototypes ======================================

// admin
void               openqueue_init(void);
bool               debugPrint_queue(void);
//remove the packets which are timeouted in the queue
void               openqueue_timeout_drop(void);
// called by any component
OpenQueueEntry_t*  openqueue_getFreePacketBuffer(uint8_t creator);
OpenQueueEntry_t*  openqueue_getFreePacketBuffer_with_timeout(uint8_t creator, const uint32_t duration_ms);
void               openqueue_set_timeout(OpenQueueEntry_t* entry, const uint32_t duration_ms);
owerror_t          openqueue_freePacketBuffer(OpenQueueEntry_t* pkt);
void               openqueue_removeEntry(OpenQueueEntry_t* entry);
void               openqueue_removeAllCreatedBy(uint8_t creator);
void               openqueue_removeAllOwnedBy(uint8_t owner);
uint8_t            openqueue_count_track(track_t track);
// called by res
OpenQueueEntry_t*  openqueue_sixtopGetSentPacket(void);
OpenQueueEntry_t*  openqueue_sixtopGetReceivedPacket(void);
//called by otf to verify the queue
OpenQueueEntry_t*    openqueue_getPacket(uint8_t pos);
// called by IEEE80215E
OpenQueueEntry_t*  openqueue_macGetDataPacket(open_addr_t* toNeighbor, track_t *track);
OpenQueueEntry_t*  openqueue_macGetEBPacket(void);
//called by openbridge
//OpenQueueEntry_t* openqueue_copy_for_openbridge(OpenQueueEntry_t* pkt);
//management
bool              openqueue_overflow_for_data(void);
//for debug
uint8_t openqueue_getPos(OpenQueueEntry_t* entry);


/**
\}
\}
*/

#endif
