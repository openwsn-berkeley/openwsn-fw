/**
\brief EV1000 definition of the "eui64" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
\author Jean-Michel Rubillon <jmrubillon@theiet.org>, September 2017.
*/

#include "string.h"
#include "eui64.h"

//=========================== defines =========================================

#define ADDRESSOFFSET 0x04
#define ADDRESS_EUI 0x1FFFF7E8

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
    
    memcpy(addressToWrite,(void const*)(ADDRESS_EUI+ADDRESSOFFSET),4);
    memcpy(addressToWrite+4,(void const*)ADDRESS_EUI,4);
  
}

//=========================== private =========================================