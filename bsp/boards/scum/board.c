/**
\brief GINA-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "memory_map.h"
#include "board.h"
#include "debugpins.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "radio.h"
#include "radiotimer.h"
#include "eui64.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
    return mote_main();
}

//=========================== public ==========================================

void board_init() {
    uint8_t eui[8];

    // initialize bsp modules
//    debugpins_init();
    leds_init();
//    uart_init();
//    radio_init();
//    radiotimer_init();

    //turn sensors off, if this is a gina (not a basestation)
//    eui64_get(eui);
}

void board_sleep() {
    // not sure how to enter a sleep mode
}

void board_reset() {
    // not sure how the reset is triggered
}

//=========================== private =========================================
