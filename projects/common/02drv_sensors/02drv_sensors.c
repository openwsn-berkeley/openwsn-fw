/**
\brief This is a program which shows how to use the "opensensors" driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

This application allows you to verify the correct functioning of the opentimers
drivers. It starts 3 periodic timers, with periods APP_DLY_TIMER0_ms,
APP_DLY_TIMER1_ms and APP_DLY_TIMER2_ms. Each timer is attached an LED (error.
radio and sync). When you run the application, you should see the LEDs
"counting".

\author Pere Tuset <peretuset@openmote.com>, January 2015.
*/

#include "stdint.h"
#include "stdio.h"

#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "scheduler.h"

#include "../../../drivers/OpenMote-CC2538/adxl346.h"
#include "../../../drivers/OpenMote-CC2538/max44009.h"
#include "../../../drivers/OpenMote-CC2538/sht21.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== main ============================================

int mote_main(void) {
    board_init();
    scheduler_init();

    // Delay needed for sensors to startup
    volatile uint32_t i;
    for (i = 0x1FFFF; i != 0; i--);

    if (adxl346_is_present()) {
        adxl346_init();
        leds_debug_on();
    }

    if (max44009_is_present()) {
        max44009_init();
        leds_sync_on();
    }

    if (sht21_is_present()) {
        sht21_init();
        leds_error_on();
    }

    scheduler_start();
    return 0;
}

//=========================== callbacks =======================================

