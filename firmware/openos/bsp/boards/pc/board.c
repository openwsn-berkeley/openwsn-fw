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

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {
   // poipoipoi stub
   printf("TODO board_init\r\n");
}

void board_sleep() {
   // poipoipoi stub
   printf("TODO board_sleep\r\n");
}

//=========================== private =========================================