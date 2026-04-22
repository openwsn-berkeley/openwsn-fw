/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "board" bsp module.
 */

#include "nrf52840.h"
#include "board.h"
#include "leds.h"
#include "sctimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "spi.h"
#include "radio.h"
#include "sensors.h"
#include "i2c.h"


//=========================== variables =======================================

//=========================== prototypes ======================================

void enable_dcdc(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
    return mote_main();
}


//=========================== public ==========================================

void board_init(void) {

    // start hfclock
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

    leds_init();
    debugpins_init();
    uart_init();
    sctimer_init();
    radio_init();

    i2c_init();

    // configure dcdc
    enable_dcdc();
}

/**
 * Puts the board to sleep
 */
void board_sleep(void) {

    __WFE();
    __WFE();
}

/**
 * Resets the board
 */
void board_reset(void) {

    NVIC_SystemReset();
}

//=========================== private =========================================

void enable_dcdc(void) {

    uint32_t status; 

    status = NRF_POWER->MAINREGSTATUS;

    if (status == 0) {

        while (NRF_POWER->DCDCEN == 0){
            // in normal voltage mode: PS1.2, page 59
            NRF_POWER->DCDCEN = (uint32_t)1;
        }
    }
}

//=========================== interrupt handlers ==============================
