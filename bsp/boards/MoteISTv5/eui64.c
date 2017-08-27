/**
\brief MoteISTv5-specific definition of the "eui64.c" bsp module.

\author Diogo Guerra <diogoguerra@ist.utl.pt>, May 2016.
*/

//=========================== includes ========================================

#include "hal_MoteISTv5.h"
#include "info_memory.h"
#include "string.h"
#include "eui64.h"

//=========================== defines =========================================

//=========================== enums ===========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
   mrm_info_table *mrm_info = INFOA;
   uint64_t eui_64 = mrm_info->eui_64;
   int n;

   addressToWrite[7] = (eui_64 & 0xFF);
   addressToWrite[6] = ((eui_64>>8) & 0xFF);
   addressToWrite[5] = ((eui_64>>16) & 0xFF);
   addressToWrite[4] = ((eui_64>>24) & 0xFF);
   addressToWrite[3] = ((eui_64>>32) & 0xFF);
   addressToWrite[2] = ((eui_64>>40) & 0xFF);
   addressToWrite[1] = ((eui_64>>48) & 0xFF);
   addressToWrite[0] = ((eui_64>>56) & 0xFF);
}

//=========================== private =========================================
