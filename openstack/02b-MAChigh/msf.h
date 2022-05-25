#ifndef OPENWSN_MSF_H
#define OPENWSN_MSF_H

/**
\addtogroup MAChigh
\{
\addtogroup msf
\{
*/

#include "opendefs.h"
#include "opentimers.h"
#include "schedule.h"
//=========================== define ==========================================

#define IANA_6TISCH_SFID_MSF    0
#define CELLOPTIONS_MSF         CELLOPTIONS_TX
#define NUMCELLS_MSF            1

#ifndef MSF_MAX_NUMCELLS
#define MAX_NUMCELLS                   32
#else
#define MAX_NUMCELLS                   MSF_MAX_NUMCELLS
#endif

#ifndef MSF_LIM_NUMCELLSUSED_HIGH
#define LIM_NUMCELLSUSED_HIGH          24
#else
#define LIM_NUMCELLSUSED_HIGH          MSF_LIM_NUMCELLSUSED_HIGH
#endif

#ifndef MSF_LIM_NUMCELLSUSED_LOW
#define LIM_NUMCELLSUSED_LOW           8
#else
#define LIM_NUMCELLSUSED_LOW           MSF_LIM_NUMCELLSUSED_LOW
#endif

#define HOUSEKEEPING_PERIOD           5000 // miliseconds
#define QUARANTINE_DURATION            300 // seconds
#define WAITDURATION_MIN             30000 // miliseconds
#define WAITDURATION_RANDOM_RANGE    30000 // miliseconds

//=========================== typedef =========================================

typedef struct {
    bool f_hashCollision;
    uint8_t backoff;
    uint8_t numCellsElapsed_tx;
    uint8_t numCellsUsed_tx;
    uint8_t numCellsElapsed_rx;
    uint8_t numCellsUsed_rx;
    opentimers_id_t housekeepingTimerId;
    uint16_t housekeepingPeriod;
    opentimers_id_t waitretryTimerId;
    bool waitretry;
    bool needAddTx;
    bool needAddRx;
    bool needDeleteTx;
    bool needDeleteRx;
    // for msf status report
    uint8_t previousNumCellsUsed_tx;
    uint8_t previousNumCellsUsed_rx;
} msf_vars_t;

typedef struct {
    uint8_t numCellsUsed_tx;
    uint8_t numCellsUsed_rx;
} msf_vars_debug_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void msf_init(void);

uint8_t msf_getsfid(void);

bool msf_candidateAddCellList(
        cellInfo_ht *cellList,
        uint8_t requiredCells
);

bool msf_candidateRemoveCellList(
        cellInfo_ht *cellList,
        open_addr_t *neighbor,
        uint8_t requiredCells,
        uint8_t cellOptions
);

// called by schedule
void msf_updateCellsElapsed(open_addr_t *neighbor, cellType_t cellType);

void msf_updateCellsUsed(open_addr_t *neighbor, cellType_t cellType);

uint16_t msf_hashFunction_getSlotoffset(open_addr_t *address);

uint8_t msf_hashFunction_getChanneloffset(open_addr_t *address);

void msf_setHashCollisionFlag(bool isCollision);

bool msf_getHashCollisionFlag(void);

uint8_t msf_getPreviousNumCellsUsed(cellType_t cellType);

bool debugPrint_msf(void);
/**
\}
\}
*/

#endif /* OPENWSN_MSF_H */
