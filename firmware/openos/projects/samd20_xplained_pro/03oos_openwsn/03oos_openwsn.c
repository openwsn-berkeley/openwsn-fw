/**
\brief This project runs the full OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "board.h"
#include "scheduler.h"
#include "openwsn.h"


int mote_main() {
	board_init();
	scheduler_init();
	openwsn_init();
	scheduler_start();
	return 0; // this line should never be reached
}