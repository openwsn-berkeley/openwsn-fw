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

/**
\brief The length of the superframe, in slots.

The superframe repears over time and can be arbitrarly long.
*/
#define SUPERFRAME_LENGTH    9

#define NUMADVSLOTS          1
#define NUMSHAREDTXRX        4
#define NUMSERIALRX          3

/**
\brief Maximum number of active slots in a superframe.

Note that this is merely used to allocate RAM memory for the schedule. The
schedule is represented, in RAM, by a table. There is one row per active slot
in that table; a slot is "active" when it is not of type CELLTYPE_OFF.

Set this number to the exact number of active slots you are planning on having
in your schedule, so not to waste RAM.
*/
#define MAXACTIVESLOTS       (NUMADVSLOTS+NUMSHAREDTXRX+NUMSERIALRX)

/**
\brief Minimum backoff exponent.

Backoff is used only in slots that are marked as shared in the schedule. When
not shared, the mote assumes that schedule is collision-free, and therefore
does not use any backoff mechanism when a transmission fails.
*/
#define MINBE                2

/**
\brief Maximum backoff exponent.

See MINBE for an explanation of backoff.
*/
#define MAXBE                4

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

PRAGMA(pack(1));
typedef struct {
   slotOffset_t    slotOffset;
   cellType_t      type;
   bool            shared;
   uint8_t         channelOffset;
   open_addr_t     neighbor;
   uint8_t         numRx;
   uint8_t         numTx;
   uint8_t         numTxACK;
   asn_t           lastUsedAsn;
   void*           next;
} scheduleEntry_t;
PRAGMA(pack());

//used to debug through ipv6 pkt. 

PRAGMA(pack(1));
typedef struct {
   uint8_t last_addr_byte;//last byte of the address; poipoi could be [0]; endianness
   uint8_t slotOffset;
   uint8_t channelOffset;
}netDebugScheduleEntry_t;
PRAGMA(pack());

PRAGMA(pack(1));
typedef struct {
   uint8_t         row;
   slotOffset_t    slotOffset;
   uint8_t         type;
   bool            shared;
   uint8_t         channelOffset;
   open_addr_t     neighbor;
   uint8_t         numRx;
   uint8_t         numTx;
   uint8_t         numTxACK;
   asn_t           lastUsedAsn;
} debugScheduleEntry_t;
PRAGMA(pack());

//=========================== module variables ================================

typedef struct {
   scheduleEntry_t  scheduleBuf[MAXACTIVESLOTS];
   scheduleEntry_t* currentScheduleEntry;
   uint16_t         frameLength;
   uint8_t          backoffExponent;
   uint8_t          backoff;
   slotOffset_t     debugPrintRow;
} schedule_vars_t;

typedef struct {
   uint8_t          numActiveSlotsCur;
   uint8_t          numActiveSlotsMax;
} schedule_dbg_t;

//=========================== prototypes ======================================

// admin
void               schedule_init();
bool               debugPrint_schedule();
bool               debugPrint_backoff();
// from uRES
void               schedule_setFrameLength(frameLength_t newFrameLength);
void               schedule_addActiveSlot(
                        slotOffset_t    slotOffset,
                        cellType_t      type,
                        bool            shared,
                        channelOffset_t channelOffset,
                        open_addr_t*    neighbor
                   );
// from IEEE802154E
void               schedule_syncSlotOffset(slotOffset_t targetSlotOffset);
void               schedule_advanceSlot();
slotOffset_t       schedule_getNextActiveSlotOffset();
frameLength_t      schedule_getFrameLength();
cellType_t         schedule_getType();
void               schedule_getNeighbor(open_addr_t* addrToWrite);
channelOffset_t    schedule_getChannelOffset();
bool               schedule_getOkToSend();
void               schedule_resetBackoff();
void               schedule_indicateRx(asn_t*   asnTimestamp);
void               schedule_indicateTx(
                        asn_t*    asnTimestamp,
                        bool      succesfullTx
                   );
void               schedule_getNetDebugInfo(netDebugScheduleEntry_t* schlist);

/**
\}
\}
*/
          
#endif
