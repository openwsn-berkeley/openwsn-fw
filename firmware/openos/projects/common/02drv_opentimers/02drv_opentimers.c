/**
\brief This is a program which shows how to use the "opentimers "driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
// driver modules required
#include "opentimers.h"
#include "debugpins.h"

//=========================== defines =========================================

#define APP_DLY_TIMER0_ms   400
#define APP_DLY_TIMER1_ms   800
#define APP_DLY_TIMER2_ms  1600

//=========================== variables =======================================

/*
typedef struct {
   
} app_vars_t;

app_vars_t app_vars;
*/

//=========================== prototypes ======================================

void cb_timer0();
void cb_timer1();
void cb_timer2();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main() {
   board_init();
   opentimers_init();
   
   opentimers_start(APP_DLY_TIMER0_ms,
                    TIMER_PERIODIC,TIME_MS,
                    cb_timer0);
   
   opentimers_start(APP_DLY_TIMER1_ms,
                    TIMER_PERIODIC,TIME_MS,
                    cb_timer1);
   
   opentimers_start(APP_DLY_TIMER2_ms,
                    TIMER_PERIODIC,TIME_MS,
                    cb_timer2);
   
   while(1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_timer0() {
   leds_error_toggle();
   debugpins_frame_toggle();
}

void cb_timer1() {
   leds_radio_toggle();
   debugpins_fsm_toggle();
}

void cb_timer2() {
   leds_sync_toggle();
   debugpins_slot_toggle();
}