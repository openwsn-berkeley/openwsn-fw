/**
\brief SCuM-specific definition of the "eui64" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/

#include "string.h"
#include "eui64.h"

//=========================== defines =========================================

//=========================== variables =======================================

const uint8_t eui64[8] = {0x00,0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
   memcpy(addressToWrite,(void const*)&eui64,8);
}

//=========================== private =========================================