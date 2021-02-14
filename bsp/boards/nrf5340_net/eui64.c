/**
\brief nRF5340_network-specific definition of the "eui64" bsp module.

\author: Tengfei Chang <tengfei.chang@inria.fr> August 2020
*/

#include "nRF5340_network.h"
#include "eui64.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
    
    uint8_t   i;
    uint8_t   shift;

    for (i=0; i<8;i++){
        shift = i & 0x3;

        addressToWrite[i] = (uint8_t)(
                (NRF_FICR_NS->INFO.DEVICEID[i/4]>>(4*(3-shift))) & 0x000000ff
            );
    }
}

//=========================== private =========================================
