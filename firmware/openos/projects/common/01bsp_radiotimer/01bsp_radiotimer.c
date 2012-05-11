/**
\brief This program shows the use of the "timers" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "radiotimer.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uint16_t num_overflow;
   uint16_t num_compare;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_overflow();
void cb_compare();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void)
{  
   // initialize board
   board_init();
   
   // switch radio LED on
   leds_radio_on();
   
   // prepare radiotimer
   radiotimer_setOverflowCb(cb_overflow);
   radiotimer_setCompareCb(cb_compare);
   radiotimer_start(0x7fff);      // @32kHz = 1000ms
   radiotimer_schedule(0xfff);    // @32kHz =  125ms
   
   while (1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_overflow() {
   // toggle pin
   debugpins_frame_toggle();
   
   // switch radio LED on
   leds_radio_on();
   
   // increment counter
   app_vars.num_overflow++;
}

void cb_compare() {
   // toggle pin
   debugpins_fsm_toggle();
   
   // switch radio LED off
   leds_radio_off();
   
   // increment counter
   app_vars.num_compare++;
}