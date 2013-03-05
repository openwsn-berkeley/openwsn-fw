/**
\brief This project runs the full OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "board.h"
#include "scheduler.h"
#include "openwsn.h"
#include "nvic.h"

int mote_main(void) {
   board_init();
   scheduler_init();
   openwsn_init();
   
   NVIC_uart();
   NVIC_bsptimer();
   NVIC_radio();
   
   scheduler_start();
   return 0; // this line should never be reached
}