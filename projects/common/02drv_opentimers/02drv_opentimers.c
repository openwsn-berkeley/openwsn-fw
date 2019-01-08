/**
\brief This is a program which shows how to use the "opentimers" driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

This application allows you to verify the correct functioning of the opentimers
drivers. It starts 3 periodic timers, with periods TIMER0_PERIOD_MS,
TIMER1_PERIOD_MS and TIMER2_PERIOD_MS. Each timer is attached an LED (error,
sync, debug, respec.).

When you run the application, you should see the LEDs "counting".

\author Thomas Watteyne <thomas.watteyne@inria.fr>, July 2017.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
// driver modules required
#include "opentimers.h"
// kernel modules required
#include "scheduler.h"

//=========================== defines =========================================

#define TIMER0_PERIOD_MS   400
#define TIMER1_PERIOD_MS   800
#define TIMER2_PERIOD_MS  1600

//=========================== variables =======================================

typedef struct {
    opentimers_id_t timer0_id;
    opentimers_id_t timer1_id;
    opentimers_id_t timer2_id;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void timer0_cb(opentimers_id_t id);
void timer1_cb(opentimers_id_t id);
void timer2_cb(opentimers_id_t id);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {

    memset(&app_vars,0,sizeof(app_vars_t));

    board_init();
    scheduler_init();
    opentimers_init();

    app_vars.timer0_id = opentimers_create(TIMER_GENERAL_PURPOSE);
    opentimers_scheduleAbsolute    (
        app_vars.timer0_id,             // id
        TIMER0_PERIOD_MS,               // duration
        opentimers_getValue(), // reference
        TIME_MS,                        // time_type
        timer0_cb                       // callback
    );

    app_vars.timer1_id = opentimers_create(TIMER_GENERAL_PURPOSE);
    opentimers_scheduleIn    (
        app_vars.timer1_id,    // id
        TIMER1_PERIOD_MS,      // duration
        TIME_MS,               // time_type
        TIMER_PERIODIC,        // timer_type
        timer1_cb              // callback
    );

    app_vars.timer2_id = opentimers_create(TIMER_GENERAL_PURPOSE);
    opentimers_scheduleIn    (
        app_vars.timer2_id,    // id
        TIMER2_PERIOD_MS,      // duration
        TIME_MS,               // time_type
        TIMER_PERIODIC,        // timer_type
        timer2_cb              // callback
    );

    scheduler_start();

    return 0;
}

//=========================== callbacks =======================================

void timer0_cb(opentimers_id_t id) {
    leds_error_toggle();
    opentimers_scheduleAbsolute    (
        app_vars.timer0_id,             // id
        TIMER0_PERIOD_MS,               // duration
        opentimers_getValue(), // reference
        TIME_MS,                        // time_type
        timer0_cb                       // callback
    );
}

void timer1_cb(opentimers_id_t id) {
    leds_sync_toggle();
}

void timer2_cb(opentimers_id_t id) {
    leds_debug_toggle();
}

//=========================== stub functions ==================================

void sniffer_setListeningChannel(void){}