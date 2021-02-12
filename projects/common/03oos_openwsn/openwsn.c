/**
   \brief This project runs the full OpenWSN stack.

   \author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
 */

#include <stdio.h>
#include "openserial.h"
#include "board.h"
#include "scheduler.h"
#include "opendrivers.h"
#include "openstack.h"
#include "openweb.h"
#include "openapps.h"

int mote_main(void) {

    // initialize
    board_init();
    scheduler_init();

    opendrivers_init();

    openstack_init();
    openweb_init();
    openapps_init();


    LOG_SUCCESS(COMPONENT_OPENWSN, ERR_BOOTED, (errorparameter_t) 0, (errorparameter_t) 0);

    // start
    scheduler_start();

    return 0; // this line should never be reached
}
