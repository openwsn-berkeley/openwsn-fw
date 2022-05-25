#ifndef OPENWSN_SCHEDULE_H
#define OPENWSN_SCHEDULE_H

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

The superframe reappears over time and can be arbitrarily long.
*/

#ifndef SLOTFRAME_LENGTH
#define SLOTFRAME_LENGTH    11 //should be 101
#endif

//draft-ietf-6tisch-minimal-06
#ifndef SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS
#define SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS                      1
#endif
#define SCHEDULE_MINIMAL_6TISCH_SLOTOFFSET                        0
#define SCHEDULE_MINIMAL_6TISCH_CHANNELOFFSET                     0
#define SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE          0 //id of slotframe
#define SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_NUMBER          1 //1 slotframe by default.

/*
  NUMSLOTSOFF is the max number of cells that the mote can add into schedule,
  besides 6TISCH_ACTIVE_CELLS and NUMSERIALRX Cell. Initially those cells are
  off. The value of NUMSLOTSOFF can be changed but the value should satisfy:

        MAXACTIVESLOTS < SLOTFRAME_LENGTH

  This would make sure number of slots are available (SLOTFRAME_LENGTH-MAXACTIVESLOTS)
  for serial port to transmit data to dagroot.
*/

#ifndef NUMSLOTSOFF
#define NUMSLOTSOFF          20
#endif

/**
\brief Maximum number of active slots in a superframe.

Note that this is merely used to allocate RAM memory for the schedule. The
schedule is represented, in RAM, by a table. There is one row per active slot
in that table; a slot is "active" when it is not of type CELLTYPE_OFF.

Set this number to the exact number of active slots you are planning on having
in your schedule, so not to waste RAM.
*/
#ifndef MAXACTIVESLOTS
#define MAXACTIVESLOTS       SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS+NUMSLOTSOFF
#endif

/**
\brief Maximum number of alternative slots (more than one cells with same slotOffset)


Note that for each slot entry, it has a table of alternative slots. All
those slots's next pointer is pointing to the same entries.

*/
#ifndef MAXBACKUPSLOTS
#define MAXBACKUPSLOTS   2
#endif

/**
\brief Minimum backoff exponent.

Backoff is used only in slots that are marked as shared in the schedule. When
not shared, the mote assumes that schedule is collision-free, and therefore
does not use any backoff mechanism when a transmission fails.
*/
#define MINBE                2 // the standard compliant range of MAXBE is 0-MAXBE

/**
\brief Maximum backoff exponent.

See MINBE for an explanation of backoff.
*/
#define MAXBE                5 // the standard compliant range of MAXBE is 3-8

/**
\brief a threshold used for triggering the maintaining process.uint: percent
*/
#define RELOCATE_PDRTHRES  30 // 50 means 50%

typedef enum {
    CELLOPTIONS_TX = 1 << 0,
    CELLOPTIONS_RX = 1 << 1,
    CELLOPTIONS_SHARED = 1 << 2,
    CELLOPTIONS_TIMEKEPPING = 1 << 3,
    CELLOPTIONS_PRIORITY = 1 << 4
} linkOptions_t;

//=========================== typedef =========================================

typedef uint8_t channelOffset_t;
typedef uint16_t slotOffset_t;
typedef uint16_t frameLength_t;

typedef enum {
    CELLTYPE_OFF = 0,
    CELLTYPE_TX = 1,
    CELLTYPE_RX = 2,
    CELLTYPE_TXRX = 3
} cellType_t;

typedef struct {
    cellType_t type;
    bool shared;
    bool isAutoCell;
    uint8_t channelOffset;
    open_addr_t neighbor;
    uint8_t numRx;
    uint8_t numTx;
    uint8_t numTxACK;
    asn_t lastUsedAsn;
    void *next;
} backupEntry_t;

typedef struct {
    slotOffset_t slotOffset;
    cellType_t type;
    bool shared;
    bool isAutoCell;
    uint8_t channelOffset;
    open_addr_t neighbor;
    uint8_t numRx;
    uint8_t numTx;
    uint8_t numTxACK;
    asn_t lastUsedAsn;
    backupEntry_t backupEntries[MAXBACKUPSLOTS];
    void *next;
} scheduleEntry_t;

BEGIN_PACK
typedef struct {
    uint8_t row;
    slotOffset_t slotOffset;
    uint8_t type;
    bool shared;
    uint8_t channelOffset;
    open_addr_t neighbor;
    uint8_t numRx;
    uint8_t numTx;
    uint8_t numTxACK;
    asn_t lastUsedAsn;
} debugScheduleEntry_t;
END_PACK

typedef struct {
    open_addr_t address;
    cellType_t link_type;
    bool shared;
    slotOffset_t slotOffset;
    channelOffset_t channelOffset;
    bool isAutoCell;
} slotinfo_element_t;

//=========================== variables =======================================

typedef struct {
    scheduleEntry_t scheduleBuf[MAXACTIVESLOTS];
    scheduleEntry_t *currentScheduleEntry;
    frameLength_t frameLength;
    frameLength_t maxActiveSlots;
    uint8_t frameHandle;
    uint8_t frameNumber;
    uint8_t backoffExponenton;
    uint8_t backoff;
    uint8_t debugPrintRow;
} schedule_vars_t;

//=========================== prototypes ======================================

// admin
void schedule_init(void);

void schedule_startDAGroot(void);

bool debugPrint_schedule(void);

bool debugPrint_backoff(void);

// from 6top
void schedule_setFrameLength(frameLength_t newFrameLength);

void schedule_setFrameHandle(uint8_t frameHandle);

void schedule_setFrameNumber(uint8_t frameNumber);

owerror_t schedule_addActiveSlot(
        slotOffset_t slotOffset,
        cellType_t type,
        bool shared,
        bool isAutoCell,
        uint8_t channelOffset,
        open_addr_t *neighbor
);

void schedule_getSlotInfo(
        slotOffset_t slotOffset,
        slotinfo_element_t *info
);

owerror_t schedule_removeActiveSlot(
        slotOffset_t slotOffset,
        cellType_t type,
        bool isShared,
        open_addr_t *neighbor
);

void schedule_removeAllAutonomousTxRxCellUnicast(void);

bool schedule_isSlotOffsetAvailable(uint16_t slotOffset);

void schedule_removeAllNegotiatedCellsToNeighbor(uint8_t slotframeID, open_addr_t *neighbor);

uint8_t schedule_getNumberOfFreeEntries(void);

uint8_t schedule_getNumberOfNegotiatedCells(open_addr_t *neighbor, cellType_t cell_type);

bool schedule_isNumTxWrapped(open_addr_t *neighbor);

bool schedule_getCellsToBeRelocated(open_addr_t *neighbor, cellInfo_ht *celllist);

bool schedule_hasAutonomousTxRxCellUnicast(open_addr_t *neighbor);

bool schedule_getAutonomousTxRxCellUnicastNeighbor(open_addr_t *neighbor);

bool schedule_hasAutoTxCellToNeighbor(open_addr_t *neighbor);

bool schedule_hasNegotiatedCellToNeighbor(open_addr_t *neighbor, cellType_t cell_type);

bool schedule_getAutonomousTxRxCellAnycast(uint16_t *slotoffset);

bool schedule_hasNonParentManagedTxCell(open_addr_t *neighbor);

void schedule_hasNegotiatedTxCell(open_addr_t *address);

bool schedule_hasNegotiatedTxCellToNonParent(open_addr_t *parentNeighbor, open_addr_t *nonParentNeighbor);

// from IEEE802154E
void schedule_syncSlotOffset(slotOffset_t targetSlotOffset);

void schedule_advanceSlot(void);

slotOffset_t schedule_getNextActiveSlotOffset(void);

frameLength_t schedule_getFrameLength(void);

cellType_t schedule_getType(void);

bool schedule_getShared(void);

bool schedule_getIsAutoCell(void);

void schedule_getNeighbor(open_addr_t *addrToWrite);

slotOffset_t schedule_getSlottOffset(void);

channelOffset_t schedule_getChannelOffset(void);

bool schedule_getOkToSend(void);

void schedule_resetBackoff(void);

void schedule_indicateRx(asn_t *asnTimestamp);

void schedule_indicateTx(asn_t *asnTimestamp, bool succesfullTx);

// from sixtop
bool schedule_getOneCellAfterOffset(
        uint8_t metadata,
        uint8_t offset,
        open_addr_t *neighbor,
        uint8_t cellOptions,
        uint16_t *slotoffset,
        uint16_t *channeloffset
);
/**
\}
\}
*/

#endif /* OPENWSN_SCHEDULE_H */
