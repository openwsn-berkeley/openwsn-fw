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
__no_init volatile uint8_t eui64 @ 0x1FFFF7E8;

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
  //We have not written ID in stm32f103re yet,
  // we write it by program temporarily.
  memcpy(addressToWrite,(void const*)(&eui64+ADDRESSOFFSET),4);
  memcpy(addressToWrite+4,(void const*)&eui64,4);
  addressToWrite[0]=0x14;
  addressToWrite[1]=0x15;
  addressToWrite[2]=0x92;
  
//  uint8_t temp_addressToWrite[64];
//  temp_addressToWrite[0] = 14;
//  temp_addressToWrite[1] = 15;
//  temp_addressToWrite[2] = 92;
//  temp_addressToWrite[3] = 0x09;
//  temp_addressToWrite[4] = 0x02;
//  temp_addressToWrite[5] = 0x2c;
//  temp_addressToWrite[6] = 0x00;
//  temp_addressToWrite[7] = 0xdf;
//  memcpy(addressToWrite,temp_addressToWrite,8);
}

//=========================== private =========================================