#ifndef __SF0_H
#define __SF0_H

/**
\addtogroup MAChigh
\{
\addtogroup sf0
\{
*/

#include "opendefs.h"
#include "opentimers.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
   uint8_t numAppPacketsPerSlotFrame;
   uint8_t backoff;
   // sfcontrol
   uint16_t sf_control_slotoffset;
   opentimers_id_t query_timer;
   opentimers_id_t trafficcontrol_timer;
   opentimers_id_t housekeeping_timer;
   bool sf_controlSlot_reserved;
   bool sf_isBusySendingQuery;
   uint8_t sf_query_factor;
   bool received6Ppreviously;
   bool controlCellConflictWithParent;
   bool busySendingKA;
   // sfcontrol
} sf0_vars_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void      sf0_init(void);
// notification from schedule
void      sf0_notifyNewSlotframe(void);
void      sf0_appPktPeriod(uint8_t numAppPacketsPerSlotFrame);

void      sf0_setBackoff(uint8_t value);
uint8_t   sf0_getsfid(void);

// sf control
uint16_t  sf0_getControlslotoffset(void);
uint16_t  sf0_hashFunction(uint16_t functionInput);
bool      sf0_isTrafficControlled(void);
void      sf0_6pQuery_notifyReceived(uint16_t query_factor, open_addr_t* neighbor);
bool      sf0_getControlslotConflictWithParent(void);
void      sf0_setControlslotConflictWithParent(bool isConflicted);

void      sf0_6pQuery_sendDone(void);
void      sf0_setReceived6Ppreviously(bool received);
// sf control

bool sf0_candidateAddCellList(
   cellInfo_ht*         cellList,
   uint8_t              requiredCells
);
bool sf0_candidateRemoveCellList(
   cellInfo_ht*         cellList,
   open_addr_t*         neighbor,
   uint8_t              requiredCells
);
/**
\}
\}
*/

#endif
