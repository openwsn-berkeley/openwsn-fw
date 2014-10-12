/**
\brief definition of the "eui64" bsp module. represented as an array of 8 bytes

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */
#include "eui64.h"
#include "iap.h"

#define OPENWSN_EUI_B0 14
#define OPENWSN_EUI_B1 15
#define OPENWSN_EUI_B2 92
#define EUI_SIZE_IN_BYTES 8

 void eui64_get(uint8_t* addressToWrite){
	uint32_t aux=0;


	IAP_return_t iap_return;
	iap_return = iapReadSerialNumber();
	memset(addressToWrite,0,8);

	if (iap_return.ReturnCode == 0)
	{
		*addressToWrite=OPENWSN_EUI_B0;
		addressToWrite++;
		*addressToWrite=OPENWSN_EUI_B1;
		addressToWrite++;
		*addressToWrite=OPENWSN_EUI_B2;
		addressToWrite++;

		aux=iap_return.Result[0];
		*addressToWrite=(uint8_t)((aux>>24) & 0x000000FF);
		addressToWrite++;
		*addressToWrite=(uint8_t)((aux>>16) & 0x000000FF);
		addressToWrite++;
		*addressToWrite=(uint8_t)((aux>>8) & 0x000000FF);
		addressToWrite++;
		*addressToWrite=(uint8_t)((aux>>0) & 0x000000FF);
		addressToWrite++;
		aux=iap_return.Result[1];
		*addressToWrite=(uint8_t)((aux>>24) & 0x000000FF);

	}


}
