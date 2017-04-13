/**
\brief This is a program which shows how to use the "opentimers2 "driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

This application allows you to verify the correct functioning of the opentimers
drivers. It starts 3 periodic timers, with periods APP_DLY_TIMER0_ms,
APP_DLY_TIMER1_ms and APP_DLY_TIMER2_ms. Each timer is attached an LED (error.
radio and sync). When you run the application, you should see the LEDs
"counting".

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
#include "sctimer.h"
// driver modules required
#include "opentimers2.h"

//=========================== defines =========================================

#define APP_DLY_TIMER0_ms   400
#define APP_DLY_TIMER1_ms   800
#define APP_DLY_TIMER2_ms  1600

//=========================== variables =======================================

typedef struct {
    opentimer2_id_t timer0;
    opentimer2_id_t timer1;
    opentimer2_id_t timer2;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_timer0(opentimer2_id_t id);
void cb_timer1(opentimer2_id_t id);
void cb_timer2(opentimer2_id_t id);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
    uint32_t reference; // reference
    
    memset(&app_vars,0,sizeof(app_vars_t));
    board_init();
    opentimer2_init();
   
    reference       = sctimer_readCounter();
    app_vars.timer0 = opentimer2_create();
    opentimer2_scheduleAbsolute(
        app_vars.timer0,       // timerId
        APP_DLY_TIMER0_ms,     // duration
        reference,             // reference
        TIME_MS,               // timetype
        cb_timer0              // callback
    );
   
    app_vars.timer1 = opentimer2_create();
    opentimer2_scheduleAbsolute(
        app_vars.timer1,       // timerId
        APP_DLY_TIMER1_ms,     // duration
        reference,             // reference
        TIME_MS,               // timetype
        cb_timer1              // callback
    );
   
    app_vars.timer2 = opentimer2_create();
    opentimer2_scheduleAbsolute(
        app_vars.timer2,       // timerId
        APP_DLY_TIMER2_ms,     // duration
        reference,             // reference
        TIME_MS,               // timetype
        cb_timer2              // callback
    );
   
    while(1) {
        board_sleep();
    }
}

//=========================== callbacks =======================================

void cb_timer0(opentimer2_id_t id) {
    leds_error_toggle();
    // re-schedule refer to previous scheduled value
    opentimer2_scheduleRelative(
        app_vars.timer0,       // timerId
        APP_DLY_TIMER2_ms,     // duration
        TIME_MS,               // timetype
        cb_timer0              // callback
    );
}

void cb_timer1(opentimer2_id_t id) {
    leds_radio_toggle();
    // re-schedule refer to previous scheduled value
    opentimer2_scheduleRelative(
        app_vars.timer1,       // timerId
        APP_DLY_TIMER2_ms,     // duration
        TIME_MS,               // timetype
        cb_timer1              // callback
    );
}

void cb_timer2(opentimer2_id_t id) {
    leds_sync_toggle();
    // re-schedule refer to previous scheduled value
    opentimer2_scheduleRelative(
        app_vars.timer2,       // timerId
        APP_DLY_TIMER2_ms,     // duration
        TIME_MS,               // timetype
        cb_timer2              // callback
    );
}