/**
\brief definition of the "eui64" bsp module. represented as an array of 8 bytes. K20 has a 32bit unique id.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
 */
#include "eui64.h"
#include "common.h"

#define OPENWSN_EUI_B0 0x14
#define OPENWSN_EUI_B1 0x15
#define OPENWSN_EUI_B2 0x92
#define EUI_SIZE_IN_BYTES 8

/**
 * Note that the UID provided by k20 is a 128 bytes unique identifier. 
 * The uid is constructed here by appending the fixed openwsn id numbers 14b 15b 92b with the lowest 8 bytes
 * mid low UID register and the four  8 bytes groups of the Low UID register. This combination can lead to a non unique number.
 */

void eui64_get(uint8_t* addressToWrite){
	uint32_t aux;
	uint32_t /*euiH,euiL,*/euiMH,euiML;
	//read SIM uid registers.

	//euiH=SIM_UIDH;
	euiMH=SIM_UIDMH;
	euiML=SIM_UIDML;
	//euiL=SIM_UIDL;

	memset(addressToWrite,0,8);


	*addressToWrite=OPENWSN_EUI_B0;
	addressToWrite++;
	*addressToWrite=OPENWSN_EUI_B1;
	addressToWrite++;
	*addressToWrite=OPENWSN_EUI_B2;
	addressToWrite++;
	//
	aux=euiML;//last byte of the Mid low eui
	*addressToWrite=(uint8_t)((aux>>0) & 0x000000FF);
	addressToWrite++;
	
	aux=euiMH;//all 4 bytes of the mid high eui
	*addressToWrite=(uint8_t)((aux>>24) & 0x000000FF);
	addressToWrite++;
	*addressToWrite=(uint8_t)((aux>>16) & 0x000000FF);
	addressToWrite++;
	*addressToWrite=(uint8_t)((aux>>8) & 0x000000FF);
	addressToWrite++;
	*addressToWrite=(uint8_t)((aux>>0) & 0x000000FF);
	//addressToWrite++;

}
