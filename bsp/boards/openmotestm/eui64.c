/**
\brief GINA-specific definition of the "eui64" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "string.h"
#include "eui64.h"

//=========================== defines =========================================

#define ADDRESSOFFSET 0x04

//=========================== variables =======================================

// address is flash where GINA's EUI64 identifier is stored
__no_init volatile uint8_t eui64 @ 0x0807FFF0;

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
  
  memcpy(addressToWrite,(void const*)&eui64,4);
  memcpy(addressToWrite+4,(void const*)(&eui64+ADDRESSOFFSET),4);
  
}

//=========================== private =========================================