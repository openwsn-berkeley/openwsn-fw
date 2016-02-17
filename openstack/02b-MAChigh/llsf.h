#ifndef __LLSF_H
#define __LLSF_H

/**
\addtogroup MAChigh
\{
\addtogroup llsf
\{
*/

#include "opendefs.h"
#include "processIE.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void llsf_init(void);

// notification from sixtop
bool llsf_candidateAddCellList(
   uint8_t*             type,
   uint8_t*             frameID,
   uint8_t*             flag,
   cellInfo_ht*         cellList
);

bool llsf_candidateRemoveCellList(
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
