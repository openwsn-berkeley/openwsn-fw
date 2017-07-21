/**
\brief SCuM-specific definition of the "board" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/

#include "memory_map.h"
#include "board.h"
#include "debugpins.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "bsp_timer.h"
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
    debugpins_init();
    leds_init();
    uart_init();
    bsp_timer_init();
    radio_init();
    radiotimer_init();
    eui64_get(eui);
}

void board_sleep() {
    // not sure how to enter a sleep mode
}

void board_reset() {
    // not sure how the reset is triggered
}

//=========================== private =========================================
