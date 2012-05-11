/**
\brief This program shows the use of both the "bsp_timer" and "radiotimer"
       bsp modules.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2012.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "bsp_timer.h"
#include "radiotimer.h"

//=========================== defines =========================================

#define BSP_TIMER_PERIOD     0x7fff // @32kHz = 1s

//=========================== variables =======================================

typedef struct {
   uint16_t num_compare;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_bsp_timer_compare();
void cb_radiotimer_overflow();
void cb_radiotimer_compare();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void)
{  
   // initialize board
   board_init();
   
   // prepare bsp_timer
   bsp_timer_set_callback(cb_bsp_timer_compare);
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
   // prepare radiotimer
   radiotimer_setOverflowCb(cb_radiotimer_overflow);
   radiotimer_setCompareCb(cb_radiotimer_compare);
   radiotimer_start(0x7fff);      // @32kHz = 1000ms
   radiotimer_schedule(0xfff);    // @32kHz =  125ms
   
   // prepare radiotimer
   
   while (1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_bsp_timer_compare() {
   // toggle pin
   debugpins_fsm_toggle();
   
   // toggle error led
   leds_error_toggle();
   
   // increment counter
   app_vars.num_compare++;
   
   // schedule again
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
}

void cb_radiotimer_overflow() {
   // toggle pin
   debugpins_frame_toggle();
   
   // switch radio LED on
   leds_radio_on();
}

void cb_radiotimer_compare() {
   // toggle pin
   debugpins_fsm_toggle();
   
   // switch radio LED off
   leds_radio_off();
}