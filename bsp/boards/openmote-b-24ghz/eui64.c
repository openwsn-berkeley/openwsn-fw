/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "eui64" bsp module.
 */

#include <string.h>

#include "eui64.h"

//=========================== defines =========================================

#define BSP_EUI64_ADDRESS_HI_H      ( 0x0028002F )
#define BSP_EUI64_ADDRESS_HI_L      ( 0x0028002C )
#define BSP_EUI64_ADDRESS_LO_H      ( 0x0028002B )
#define BSP_EUI64_ADDRESS_LO_L      ( 0x00280028 )

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
   uint8_t* eui64_flash;
   
   eui64_flash = (uint8_t*)BSP_EUI64_ADDRESS_LO_H;
   while(eui64_flash >= (uint8_t*)BSP_EUI64_ADDRESS_LO_L) {
      *addressToWrite++ = *eui64_flash--;
   }
   
   eui64_flash = (uint8_t*)BSP_EUI64_ADDRESS_HI_H;
   while(eui64_flash >= (uint8_t*)BSP_EUI64_ADDRESS_HI_L) {
      *addressToWrite++ = *eui64_flash--;
   }
}

//=========================== private =========================================

