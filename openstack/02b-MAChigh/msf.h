#ifndef __MSF_H
#define __MSF_H

/**
\addtogroup MAChigh
\{
\addtogroup msf
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

#define CELL_USAGE_CALCULATION_WINDOWS 16

//=========================== typedef =========================================

typedef struct {
   uint8_t numAppPacketsPerSlotFrame;
   uint8_t backoff;
} msf_vars_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void      msf_init(void);
// notification from schedule
void      msf_notifyNewSlotframe(void);
void      msf_appPktPeriod(uint8_t numAppPacketsPerSlotFrame);

void      msf_setBackoff(uint8_t value);
uint8_t   msf_getsfid(void);

bool msf_candidateAddCellList(
   cellInfo_ht*         cellList,
   uint8_t              requiredCells
);
bool msf_candidateRemoveCellList(
   cellInfo_ht*         cellList,
   open_addr_t*         neighbor,
   uint8_t              requiredCells
);
/**
\}
\}
*/

#endif
