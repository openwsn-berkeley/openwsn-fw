/**
\brief This project runs the full OpenWSN stack on the GINA2.2b/c and GINA
       basestations platforms.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "test_openwsn.h"
//board
#include "gina.h"
#include "leds.h"
//openwsn
#include "openwsn.h"
#include "scheduler.h"
#include "packetfunctions.h"
#include "openqueue.h"
#include "IEEE802154E.h"
#include "idmanager.h"
#include "openserial.h"

void main(void) {
   gina_init();
   openwsn_init();
   scheduler_start();
}