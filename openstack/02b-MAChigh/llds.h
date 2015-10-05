#ifndef __LLDS_H
#define __LLDS_H

/**
\addtogroup MAChigh
\{
\addtogroup llds
\{
*/

#include "opendefs.h"
#include "processIE.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void llds_init(void);

// notification from sixtop
bool llds_candidateAddCellList(
   uint8_t*             type,
   uint8_t*             frameID,
   uint8_t*             flag,
   cellInfo_ht*         cellList
);

bool llds_candidateRemoveCellList(
   uint8_t*             type,
   uint8_t*             frameID,
   uint8_t*             flag,
   cellInfo_ht*         cellList,
   open_addr_t*         neighbor,
   uint16_t             numOfCells
);
/**
\}
\}
*/

#endif
