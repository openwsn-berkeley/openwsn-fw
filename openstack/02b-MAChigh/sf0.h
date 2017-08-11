#ifndef __SF0_H
#define __SF0_H

/**
\addtogroup MAChigh
\{
\addtogroup sf0
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
   uint8_t numAppPacketsPerSlotFrame;
   uint8_t backoff;
   // sfcontrol
   uint16_t sf_control_slotoffset;
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
