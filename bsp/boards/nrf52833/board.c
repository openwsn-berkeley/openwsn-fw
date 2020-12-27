/**
\brief nRF52840-specific definition of the "board" bsp module.

\author Tengfei Chang <tengfei.chang@gmail.com>, July 2020.
*/

#include "nrf52833.h"
#include "board.h"
// bsp modules
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "sctimer.h"
#include "radio.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void clocks_start(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void) {

    clocks_start();

    // initialize bsp modules
    debugpins_init();
    leds_init();
    uart_init();
    radio_init();
    sctimer_init();
}

void board_sleep(void) {
    // todo
}

void board_reset(void) {
    // todo

    NVIC_SystemReset();
}

//=========================== private =========================================

void clocks_start( void ){

    // Start HFCLK and wait for it to start.
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}