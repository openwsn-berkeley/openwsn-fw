/**
\brief PC-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "board.h"
// bsp modules
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "bsp_timer.h"
#include "radio.h"
#include "radiotimer.h"
#include "eui64.h"
// OpenSim environment
#include "opensim_client.h"
#include "opensim_proto.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_board_init,
                                    0,
                                    0,
                                    0,
                                    0);
}

void board_sleep() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_board_sleep,
                                    0,
                                    0,
                                    0,
                                    0);
}

//=========================== private =========================================