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

#define SCHEDULELENGTH  9    // the length, in slot, of the schedule
#define MINBE           2    // min backoff exponent, used in shared TX slots
#define MAXBE           4    // max backoff exponent, used in shared TX slots

enum {
   CELLTYPE_OFF              = 0,
   CELLTYPE_ADV              = 1,
   CELLTYPE_TX               = 2,
   CELLTYPE_RX               = 3,
   CELLTYPE_TXRX             = 4,
   CELLTYPE_SERIALRX         = 5,
   CELLTYPE_MORESERIALRX     = 6
};

//=========================== typedef =========================================

typedef uint8_t    cellType_t;
typedef uint8_t    channelOffset_t;

typedef struct {
   uint8_t         type;
   bool            shared;
   uint8_t         backoffExponent;
   uint8_t         backoff;
   uint8_t         channelOffset;
   open_addr_t     neighbor;
   uint8_t         numRx;
   uint8_t         numTx;
   uint8_t         numTxACK;
   asn_t           asn;
} scheduleRow_t;

typedef struct {
   uint8_t         row;
   scheduleRow_t   cellUsage;
} debugScheduleRow_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
          void            schedule_init();
          bool            debugPrint_schedule();
// from IEEE802154E
__monitor cellType_t      schedule_getType(uint16_t slotOffset);
__monitor bool            schedule_getOkToSend(uint16_t slotOffset);
__monitor void            schedule_getNeighbor(uint16_t slotOffset, open_addr_t* addrToWrite);
__monitor channelOffset_t schedule_getChannelOffset(uint16_t slotOffset);
__monitor void            schedule_indicateRx(uint16_t slotOffset,
                                              asn_t*   asnTimestamp);
__monitor void            schedule_indicateTx(uint16_t slotOffset,
                                              asn_t*   asnTimestamp,
                                              bool     succesfullTx);

/**
\}
\}
*/
          
#endif