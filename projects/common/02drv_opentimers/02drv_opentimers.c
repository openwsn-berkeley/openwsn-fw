/**
\brief This is a program which shows how to use the "opentimers" driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

This application allows you to verify the correct functioning of the opentimers
drivers. It starts 3 periodic timers.
Each timer is attached an LED (error, radio and sync). When you run the application, you should see the LEDs
"counting".

|    timer value    |  LED  | debugpin |
|-------------------|-------|----------|
| APP_DLY_TIMER0_ms | error |    frame |
| APP_DLY_TIMER1_ms | radio |     slot |
| APP_DLY_TIMER2_ms |  sync |      fsm |

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
#include "debugpins.h"
// driver modules required
#include "opentimers.h"

//=========================== defines =========================================

#define APP_DLY_TIMER0_ms   40
#define APP_DLY_TIMER1_ms   80
#define APP_DLY_TIMER2_ms  160

//=========================== variables =======================================

//=========================== prototypes ======================================

void cb_timer0(opentimer_id_t id);
void cb_timer1(opentimer_id_t id);
void cb_timer2(opentimer_id_t id);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   board_init();
   opentimers_init();
   
   opentimers_start(
      APP_DLY_TIMER0_ms,     // duration
      TIMER_PERIODIC,        // type
      TIME_MS,               // timetype
      cb_timer0              // callback
   );
   
   opentimers_start(
      APP_DLY_TIMER1_ms,     // duration
      TIMER_PERIODIC,        // type
      TIME_MS,               // timetype
      cb_timer1              // callback
   );
   
   opentimers_start(
      APP_DLY_TIMER2_ms,     // duration
      TIMER_PERIODIC,        // type
      TIME_MS,               // timetype
      cb_timer2              // callback
   );
   
   while(1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_timer0(opentimer_id_t id) {
    debugpins_frame_toggle();
   leds_error_toggle();
}

void cb_timer1(opentimer_id_t id) {
   debugpins_slot_toggle();
   leds_radio_toggle();   
}

void cb_timer2(opentimer_id_t id) {
   debugpins_fsm_toggle();
   leds_sync_toggle();
}

//=========================== stub functions ==================================

void sniffer_setListeningChannel(void){}