/**
\brief nRF52840-specific definition of the "eui64" bsp module.

\author Tengfei Chang <tengfei.chang@gmail.com>, July 2020.
*/

#include "nrf52833.h"
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
                (NRF_FICR->DEVICEID[i/4]>>(4*(3-shift))) & 0x000000ff
            );
    }
}

//=========================== private =========================================
