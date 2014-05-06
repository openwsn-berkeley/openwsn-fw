/**
\brief This program shows the use of the "radiotimer" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "radiotimer.h"

//=========================== defines =========================================

#define RADIOTIMER_OVERFLOW_PERIOD     0x8000
#define RADIOTIMER_COMPARE_PERIOD      0x1000
#define RADIOTIMER_NUM_COMPARES             4

//=========================== variables =======================================

typedef struct {
   uint8_t  num_compares_left;
   uint16_t last_compare_val;
} app_vars_t;

app_vars_t app_vars;

typedef struct {
   uint16_t num_overflow;
   uint16_t num_compare;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== prototypes ======================================

void cb_overflow();
void cb_compare();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main()
{  
   // initialize board
   board_init();
   
   // switch radio LED on
   leds_radio_on();
   
   // prepare radiotimer
   radiotimer_setOverflowCb(cb_overflow);
   radiotimer_setCompareCb(cb_compare);
   
   
   // kick off first compare
   app_vars.num_compares_left  = RADIOTIMER_NUM_COMPARES-1;
   app_vars.last_compare_val   = RADIOTIMER_COMPARE_PERIOD;

   // start periodic overflow
   radiotimer_start(RADIOTIMER_OVERFLOW_PERIOD);

   radiotimer_schedule(app_vars.last_compare_val);
   
   while (1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_overflow() {
   // toggle pin
   debugpins_frame_toggle();
   
   // switch radio LED on
   leds_error_toggle();
   leds_radio_off();
   
   // reset the counter for number of remaining compares
   app_vars.num_compares_left  = RADIOTIMER_NUM_COMPARES;
   app_vars.last_compare_val   = RADIOTIMER_COMPARE_PERIOD;
   radiotimer_schedule(app_vars.last_compare_val);
   
   // increment debug counter
   app_dbg.num_overflow++;
}

void cb_compare() {
   // toggle pin
   debugpins_fsm_toggle();
   
   // toggle radio LED
   leds_radio_toggle();
   
   // schedule a next compare, if applicable
   app_vars.last_compare_val += RADIOTIMER_COMPARE_PERIOD;   
   app_vars.num_compares_left--;
   if (app_vars.num_compares_left>0) {
      radiotimer_schedule(app_vars.last_compare_val);
   } else {
      radiotimer_cancel();
   }
   
   // increment debug counter
   app_dbg.num_compare++;
}
