/**
\brief CC2538-specific definition of the "eui64" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, September 2013.
*/

#include "string.h"
#include "eui64.h"

//=========================== defines =========================================

//=========================== variables =======================================



//=========================== prototypes ======================================

//=========================== public ==========================================

//eui hardcoded by now..
void eui64_get(uint8_t* addressToWrite) {
	uint8_t temp_addressToWrite[64];
	  temp_addressToWrite[0] = 14;
	  temp_addressToWrite[1] = 15;
	  temp_addressToWrite[2] = 92;
	  temp_addressToWrite[3] = 0x09;
	  temp_addressToWrite[4] = 0x02;
	  temp_addressToWrite[5] = 0x2c;
	  temp_addressToWrite[6] = 0x00;
	  temp_addressToWrite[7] = 0xdf;

	  memcpy(addressToWrite,temp_addressToWrite,8);
}

//=========================== private =========================================
