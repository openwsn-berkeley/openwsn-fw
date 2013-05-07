/**
\brief PC-based emulation of the mote's power supply.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <stdio.h>
#include "supply.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   int numBoots;
} supply_vars_t;

supply_vars_t supply_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void supply_init() {
   
   // clear variables
   memset(&supply_vars,0,sizeof(supply_vars_t));
}

/**
\brief Root function of the emulated mote.
*/
void supply_rootFunction() {
   int rxPacketType;
   int rxPktParamsLength;
   
   printf("Waiting for boot\r\n");
   
   // wait for the supply to switch on
   /*
   opensim_client_waitForPacket(&rxPacketType,
                                0,
                                0,
                                &rxPktParamsLength);
   
   // TODO: replace by call to Python
   
   // make sure this is the supply switching on
   if (rxPacketType!=OPENSIM_CMD_supply_on) {
      fprintf(stderr,"ERROR: expected OPENSIM_CMD_supply_on, got command %d\n",
                                  rxPacketType);
      //opensim_client_abort();
      // TODO: replace by call to Python
   }
   */
   
   mote_main();
}

//=========================== interrupt handlers ==============================