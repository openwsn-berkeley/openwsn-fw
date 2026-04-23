/**
 * brief nrf52840-specific definition of the "eui64" bsp module.
*/

#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "string.h"
#include "eui64.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
    
    uint32_t tmp;
    uint8_t i;

    i = 0;

    // get ID from Nordic chip
    tmp = NRF_FICR->DEVICEID[0];
    for (i=0;i<4;i++) {
        addressToWrite[i] = (uint8_t)((tmp >> (i*8)) & 0x000000ff);
    }
    tmp = NRF_FICR->DEVICEID[1];
    for (i=0;i<4;i++) {
        addressToWrite[i+4] = (uint8_t)((tmp >> (i*8)) & 0x000000ff);
    }
}

//=========================== private =========================================