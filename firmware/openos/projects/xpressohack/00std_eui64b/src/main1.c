

/**
\brief This is a standalone test program for the IAP on the xpressohack
       board.

\author xavi vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012
*/


#include "LPC17xx.h"
#include "stdio.h"
#include "stdint.h"
#include "iap.h"
#include "eui64.h"

int main(void) {
	uint8_t* eui;
	uint8_t addr_64b[8];
	eui=&addr_64b;

	IAP_return_t iap_return;
	iap_return = iapReadSerialNumber();

	    if (iap_return.ReturnCode == 0)
	    {
	      printf("Serial Number: %08X %08X %08X %08X %s",
	              iap_return.Result[0],
	              iap_return.Result[1],
	              iap_return.Result[2],
	              iap_return.Result[3],
	              "\n");
	    }


	    eui64_get(eui);

printf("EUI64: %02X %02X %02X %02X %02X %02X %02X %02X %s",
	   eui[0],eui[1],eui[2],eui[3],eui[4],eui[5],eui[6],eui[7],"\n");


return 0;
}
