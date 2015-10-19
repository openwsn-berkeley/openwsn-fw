#ifndef __SCHEDULE_H
#define __SCHEDULE_H

/**
\addtogroup MAChigh
\{
\addtogroup Schedule
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

/**
\brief The length of the superframe, in slots.

The superframe repears over time and can be arbitrarly long.
*/
#define SLOTFRAME_LENGTH    11 //should be 101

//draft-ietf-6tisch-minimal-06
#define SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS                      1
#define SCHEDULE_MINIMAL_6TISCH_SLOTOFFSET                        0
#define SCHEDULE_MINIMAL_6TISCH_CHANNELOFFSET                     0
#define SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE          1 //id of slotframe
#define SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_NUMBER          1 //1 slotframe by default.

#define NUMSERIALRX          1
#define NUMSLOTSOFF          1

/**
\brief Maximum number of active slots in a superframe.

Note that this is merely used to allocate RAM memory for the schedule. The
schedule is represented, in RAM, by a table. There is one row per active slot
in that table; a slot is "active" when it is not of type CELLTYPE_OFF.

Set this number to the exact number of active slots you are planning on having
in your schedule, so not to waste RAM.
*/
#define MAXACTIVESLOTS       (SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS+NUMSLOTSOFF)

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

/**
\brief a threshold used for triggering the maintaining process.uint: percent
*/
#define PDR_THRESHOLD      80 // 80 means 80%
#define MIN_NUMTX_FOR_PDR  50 // don't calculate PDR when numTx is lower than this value 

//=========================== typedef =========================================

typedef uint8_t    channelOffset_t;
typedef uint16_t   slotOffset_t;
typedef uint16_t   frameLength_t;

typedef enum {
   CELLTYPE_OFF              = 0,
   CELLTYPE_TX               = 1,
   CELLTYPE_RX               = 2,
   CELLTYPE_TXRX             = 3,
   CELLTYPE_SERIALRX         = 4,
   CELLTYPE_MORESERIALRX     = 5
} cellType_t;

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

BEGIN_PACK
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
END_PACK

typedef struct {
  uint8_t          address[LENGTH_ADDR64b];
  cellType_t       link_type;
  bool             shared;
  slotOffset_t     slotOffset;
  channelOffset_t  channelOffset;
}slotinfo_element_t;

//=========================== variables =======================================

typedef struct {
   scheduleEntry_t  scheduleBuf[MAXACTIVESLOTS];
   scheduleEntry_t* currentScheduleEntry;
   frameLength_t    frameLength;
   frameLength_t    maxActiveSlots;
   uint8_t          frameHandle;
   uint8_t          frameNumber;
   uint8_t          backoffExponent;
   uint8_t          backoff;
   uint8_t          debugPrintRow;
} schedule_vars_t;

//=========================== prototypes ======================================

// admin
void               schedule_init(void);
void               schedule_startDAGroot(void);
bool               debugPrint_schedule(void);
bool               debugPrint_backoff(void);

// from 6top
void               schedule_setFrameLength(frameLength_t newFrameLength);
void               schedule_setFrameHandle(uint8_t frameHandle);
void               schedule_setFrameNumber(uint8_t frameNumber);
owerror_t          schedule_addActiveSlot(
   slotOffset_t         slotOffset,
   cellType_t           type,
   bool                 shared,
   uint8_t              channelOffset,
   open_addr_t*         neighbor
);

void               schedule_getSlotInfo(
   slotOffset_t         slotOffset,                      
   open_addr_t*         neighbor,
   slotinfo_element_t*  info
);

uint16_t           schedule_getMaxActiveSlots(void);

owerror_t          schedule_removeActiveSlot(
   slotOffset_t         slotOffset,
   open_addr_t*         neighbor
);
bool               schedule_isSlotOffsetAvailable(uint16_t slotOffset);
// return the slot info which has a poor quality
scheduleEntry_t*  schedule_statistic_poorLinkQuality(void);

// from IEEE802154E
bool               schedule_syncSlotOffset(slotOffset_t targetSlotOffset);
void               schedule_advanceSlot(void);
slotOffset_t       schedule_getNextActiveSlotOffset(void);
slotOffset_t       schedule_getClosestActiveSlotOffset(slotOffset_t targetSlotOffset);
frameLength_t      schedule_getFrameLength(void);
uint8_t            schedule_getFrameHandle(void);
uint8_t            schedule_getFrameNumber(void);
cellType_t         schedule_getType(void);
void               schedule_getNeighbor(open_addr_t* addrToWrite);
channelOffset_t    schedule_getChannelOffset(void);
bool               schedule_getOkToSend(void);
void               schedule_resetBackoff(void);
void               schedule_indicateRx(asn_t*   asnTimestamp);
void               schedule_indicateTx(
                        asn_t*    asnTimestamp,
                        bool      succesfullTx
                   );

/**
\}
\}
*/
          
#endif
