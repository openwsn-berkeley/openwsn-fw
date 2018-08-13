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
#include "radio.h"
#include "eui64.h"
#include "sctimer.h"
#include "scm3_hardware_interface.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
    return mote_main();
}

//=========================== public ==========================================

void board_init(void) {
    uint8_t eui[8];

    // initialize the scan chain
    initialize_ASC();
    
    IPR0 = 0xFF;    // uart has lowest priority
    IPR6 = 0x0F;    // priority for radio
    IPR7 = 0x00;    // priority for rf_timer
    
    // initialize bsp modules
    debugpins_init();
    leds_init();
    uart_init();
    sctimer_init();
    radio_init();
    eui64_get(eui);
}

void board_sleep(void) {
    // not sure how to enter a sleep mode
}

void board_reset(void) {
    // not sure how the reset is triggered
}

//=========================== private =========================================
