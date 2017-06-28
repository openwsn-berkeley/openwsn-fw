/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: EZR32WG-specific definition of the "eui64" bsp module.
 */

#include "string.h"
#include "eui64.h"

//=========================== defines =========================================
#define ADDRESSOFFSET   0x04
#define ADDRESS_EUI     0x0FE081F0

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {

    memcpy(addressToWrite,(void const*)ADDRESS_EUI,4);
    memcpy(addressToWrite+4,(void const*)(ADDRESS_EUI+ADDRESSOFFSET),4);
}

//=========================== private =========================================

