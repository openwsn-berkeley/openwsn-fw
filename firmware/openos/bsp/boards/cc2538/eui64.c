/**
\brief CC2538-specific definition of the "eui64" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, September 2013.
\author Pere Tuset <peretuset@uoc.edu>, December 2013.
*/

#include "string.h"
#include "eui64.h"

//=========================== defines =========================================

#define BSP_EUI64_ADDRESS_HI_H      ( 0x0028002F )
#define BSP_EUI64_ADDRESS_HI_L      ( 0x0028002C )
#define BSP_EUI64_ADDRESS_LO_H      ( 0x0028002B )
#define BSP_EUI64_ADDRESS_LO_L      ( 0x00280028 )

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

   uint8_t* eui64_flash;
    
   eui64_flash = (uint8_t*)BSP_EUI64_ADDRESS_LO_H;
   while(eui64_flash >= (uint8_t*)BSP_EUI64_ADDRESS_LO_L) {
    *addressToWrite++ = *eui64_flash--;
    }
    
    eui64_flash = (uint8_t*)BSP_EUI64_ADDRESS_HI_H;
    while(eui64_flash >= (uint8_t*)BSP_EUI64_ADDRESS_HI_L) {
       *addressToWrite++ = *eui64_flash--;
    }

//=========================== private =========================================
