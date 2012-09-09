/**
\brief This project runs the full OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "openwsn.h"
//board
#include "board.h"
#include "leds.h"
//openwsn
#include "openwsn.h"
#include "scheduler.h"
#include "packetfunctions.h"
#include "openqueue.h"
#include "IEEE802154E.h"
#include "idmanager.h"
#include "openserial.h"

int mote_main(void) {
   board_init();
   scheduler_init();
   openwsn_init();
   scheduler_start();
   return 0; // this line should never be reached
}