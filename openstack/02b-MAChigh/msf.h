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

#define MAX_NUMCELLS            16
#define LIM_NUMCELLSUSED_HIGH   12
#define LIM_NUMCELLSUSED_LOW     4

#define HOUSEKEEPING_PERIOD     60 // seconds

//=========================== typedef =========================================

typedef struct {
   uint8_t numAppPacketsPerSlotFrame;
   uint8_t backoff;
   uint8_t numCellsPassed;
   uint8_t numCellsUsed;
   opentimers_id_t housekeepingTimerId;
   uint8_t housekeepingTimerCounter;
} msf_vars_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void    msf_init(void);
void    msf_appPktPeriod(uint8_t numAppPacketsPerSlotFrame);
void    msf_setBackoff(uint8_t value);
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
void    msf_updateCellsPassed(void);
void    msf_updateCellsUsed(void);
/**
\}
\}
*/

#endif
