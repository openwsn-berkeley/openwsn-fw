#ifndef __SCHEDULE_H
#define __SCHEDULE_H

/**
\addtogroup MAChigh
\{
\addtogroup Schedule
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

#define MAXACTIVESLOTS  5    // the maximum number of active slots
#define MINBE           2    // min backoff exponent, used in shared TX slots
#define MAXBE           4    // max backoff exponent, used in shared TX slots

 
//=========================== typedef =========================================

typedef uint8_t    channelOffset_t;
typedef uint16_t   slotOffset_t;
typedef uint16_t   frameLength_t;

typedef enum {
   CELLTYPE_OFF              = 0,
   CELLTYPE_ADV              = 1,
   CELLTYPE_TX               = 2,
   CELLTYPE_RX               = 3,
   CELLTYPE_TXRX             = 4,
   CELLTYPE_SERIALRX         = 5,
   CELLTYPE_MORESERIALRX     = 6
} cellType_t;

typedef struct {
   slotOffset_t    slotOffset;
   cellType_t      type;
   bool            shared;
   uint8_t         channelOffset;
   open_addr_t     neighbor;
   uint8_t         backoffExponent;
   uint8_t         backoff;
   uint8_t         numRx;
   uint8_t         numTx;
   uint8_t         numTxACK;
   asn_t           lastUsedAsn;
   void*           next;
} scheduleEntry_t;

//used to debug through ipv6 pkt. 
typedef struct {
   uint8_t last_addr_byte;//last byte of the address; poipoi could be [0]; endianness
   uint8_t slotOffset;
   uint8_t channelOffset;
}netDebugScheduleEntry_t;



typedef struct {
   uint8_t         row;
   scheduleEntry_t scheduleEntry;
} debugScheduleEntry_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
          void            schedule_init();
          bool            debugPrint_schedule();
// from uRES
          void            schedule_setFrameLength(frameLength_t newFrameLength);
          void            schedule_addActiveSlot(slotOffset_t    slotOffset,
                                                 cellType_t      type,
                                                 bool            shared,
                                                 uint8_t         channelOffset,
                                                 open_addr_t*    neighbor);
// from IEEE802154E
 void            schedule_syncSlotOffset(slotOffset_t targetSlotOffset);
 void            schedule_advanceSlot();
 slotOffset_t    schedule_getNextActiveSlotOffset();
 frameLength_t   schedule_getFrameLength();
 cellType_t      schedule_getType();
 void            schedule_getNeighbor(open_addr_t* addrToWrite);
 channelOffset_t schedule_getChannelOffset();
 bool            schedule_getOkToSend();
 void            schedule_indicateRx(asn_t*   asnTimestamp);
 void            schedule_indicateTx(asn_t*   asnTimestamp,
                                              bool     succesfullTx);
 void            scheduleBuf_getAll(scheduleEntry_t *blist);
 void            schedule_getNetDebugInfo(netDebugScheduleEntry_t *schlist,uint8_t maxbytes);

/**
\}
\}
*/
          
#endif
