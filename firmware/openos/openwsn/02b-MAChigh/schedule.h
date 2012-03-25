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
__monitor void            schedule_setFrameLength(frameLength_t newFrameLength);
__monitor void            schedule_addActiveSlot(slotOffset_t    slotOffset,
                                                 cellType_t      type,
                                                 bool            shared,
                                                 uint8_t         channelOffset,
                                                 open_addr_t*    neighbor);
// from IEEE802154E
__monitor void            schedule_syncSlotOffset(slotOffset_t targetSlotOffset);
__monitor void            schedule_advanceSlot();
__monitor slotOffset_t    schedule_getNextActiveSlotOffset();
__monitor frameLength_t   schedule_getFrameLength();
__monitor cellType_t      schedule_getType();
__monitor void            schedule_getNeighbor(open_addr_t* addrToWrite);
__monitor channelOffset_t schedule_getChannelOffset();
__monitor bool            schedule_getOkToSend();
__monitor void            schedule_indicateRx(asn_t*   asnTimestamp);
__monitor void            schedule_indicateTx(asn_t*   asnTimestamp,
                                              bool     succesfullTx);

/**
\}
\}
*/
          
#endif