/**
\brief This is a program which shows how to use the "opentimers" driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

This application allows you to verify the correct functioning of the opentimers
drivers. It starts 3 periodic timers, with periods APP_DLY_TIMER0_ticks,
APP_DLY_TIMER1_ticks and APP_DLY_TIMER2_ticks. Each timer is attached an LED
(error, radio, sync, resp/).

When you run the application, you should see the LEDs blinking.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
\author Thomas Watteyne <thomas.watteyne@inria.fr>, July 2017.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
#include "sctimer.h"
// driver modules required
#include "opentimers.h"

//=========================== defines =========================================

#define APP_DLY_TIMER0_ticks   16384    // half second @ 32kHz
#define APP_DLY_TIMER1_ticks   16384    // a little bit more than half second @ 32kHz
#define APP_DLY_TIMER2_ticks   32768    // one  second @ 32kHz

//=========================== variables =======================================

typedef struct {
    opentimers_id_t timer0;
    opentimers_id_t timer1;
    opentimers_id_t timer2;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_timer0(opentimers_id_t id);
void cb_timer1(opentimers_id_t id);
void cb_timer2(opentimers_id_t id);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
    
    memset(&app_vars,0,sizeof(app_vars_t));
    
    board_init();
    opentimers_init();
   
    app_vars.timer0 = opentimers_create();
    opentimers_scheduleIn(
        app_vars.timer0,          // timerId
        APP_DLY_TIMER0_ticks,     // duration
        TIME_TICS,                // timetype
        TIMER_PERIODIC,           // timertype
        cb_timer0                 // callback
    );
   
    app_vars.timer1 = opentimers_create();
    opentimers_scheduleIn(
        app_vars.timer1,          // timerId
        APP_DLY_TIMER0_ticks,     // duration
        TIME_TICS,                // timetype
        TIMER_PERIODIC,           // timertype
        cb_timer1                 // callback
    );
   
    app_vars.timer2 = opentimers_create();
    opentimers_scheduleIn(
        app_vars.timer2,          // timerId
        APP_DLY_TIMER0_ticks,     // duration
        TIME_TICS,                // timetype
        TIMER_PERIODIC,           // timertype
        cb_timer2                 // callback
    );
    
    while(1) {
        board_sleep();
    }
}

//=========================== callbacks =======================================

void cb_timer0(opentimers_id_t id) {
    leds_error_toggle();
}

void cb_timer1(opentimers_id_t id) {
    leds_radio_toggle();
}

void cb_timer2(opentimers_id_t id) {
    leds_sync_toggle();
}