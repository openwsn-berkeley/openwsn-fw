#ifndef __MSF_H
#define __MSF_H

/**
\addtogroup MAChigh
\{
\addtogroup msf
\{
*/

#include "opendefs.h"
#include "opentimers.h"
//=========================== define ==========================================

#define IANA_6TISCH_SFID_MSF    0
#define CELLOPTIONS_MSF         CELLOPTIONS_TX | CELLOPTIONS_RX | CELLOPTIONS_SHARED
#define NUMCELLS_MSF            1

#define MAX_NUMCELLS                   16
#define LIM_NUMCELLSUSED_HIGH          12
#define LIM_NUMCELLSUSED_LOW            4

#define HOUSEKEEPING_PERIOD          30000 // miliseconds
#define QUARANTINE_DURATION            300 // seconds
#define WAITDURATION_MIN             30000 // miliseconds
#define WAITDURATION_RANDOM_RANGE    30000 // miliseconds

//=========================== typedef =========================================

typedef struct {
   uint8_t              numAppPacketsPerSlotFrame;
   uint8_t              backoff;
   uint8_t              numCellsPassed;
   uint8_t              numCellsUsed;
   opentimers_id_t      housekeepingTimerId;
   uint16_t             housekeepingPeriod;
   opentimers_id_t      waitretryTimerId;
   bool                 waitretry;
} msf_vars_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void    msf_init(void);
void    msf_appPktPeriod(uint8_t numAppPacketsPerSlotFrame);
uint8_t msf_getsfid(void);
bool    msf_candidateAddCellList(
    cellInfo_ht* cellList,
    uint8_t requiredCells
);
bool    msf_candidateRemoveCellList(
    cellInfo_ht* cellList,
    open_addr_t* neighbor,
    uint8_t requiredCells
);
// called by schedule
void    msf_updateCellsPassed(open_addr_t* neighbor);
void    msf_updateCellsUsed(open_addr_t* neighbor);
// called by icmpv6rpl, where parent changed
void    msf_trigger6pClear(open_addr_t* neighbor);
/**
\}
\}
*/

#endif
