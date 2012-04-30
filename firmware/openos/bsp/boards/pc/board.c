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

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {
   //poipoiopensim_client_send(0);
}

void board_sleep() {
   //poipoiopensim_client_send(1);
}

//=========================== private =========================================