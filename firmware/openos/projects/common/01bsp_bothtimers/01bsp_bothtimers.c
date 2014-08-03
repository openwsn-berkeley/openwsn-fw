/**
\brief This program shows the use of both the "bsp_timer" and "radiotimer"
       bsp modules.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

Load this program onto your board, and start running. It will enable the BSP
timer and radiotimer:
- the BSP timer is periodic, of period BSP_TIMER_PERIOD ticks. Each time it
  elapses:
    - the frame debugpin toggles
    - the sync LED toggles
- the radiotimer:
    - overflows every RADIOTIMER_OVERFLOW_PERIOD
    - during that period, RADIOTIMER_NUM_COMPARES compare events fire,
      every RADIOTIMER_COMPARE_PERIOD ticks.

The resulting signal of the radiotimer is:

   |         RADIOTIMER_OVERFLOW_PERIOD       |
   |<---------------------------------------->|
   |                                          |
   v                                          v
overflow                                  overflow

   :      RADIOTIMER_COMPARE_PERIOD
   :              >---------<
   :       |       |       |       |
   :       |       |       |       |
   :       v       v       v       v
   :    compare compare compare compare

- At each radiotimer overflow:
    - the slot debugpin toggles
    - the error LED toggles
- At each radiotimer compare:
    - the fsm debugpin toggles
    - the radio LED toggles

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "bsp_timer.h"
#include "radiotimer.h"

//=========================== defines =========================================

#define BSP_TIMER_PERIOD                  50000
#define RADIOTIMER_OVERFLOW_PERIOD        32768
#define RADIOTIMER_COMPARE_PERIOD          5000
#define RADIOTIMER_NUM_COMPARES               4
#define ISR_DELAY                           250

//=========================== variables =======================================

typedef struct {
   uint8_t  radiotimer_num_compares_left;
   uint16_t radiotimer_last_compare_val;
} app_vars_t;

app_vars_t app_vars;

typedef struct {
   uint16_t bsp_timer_num_compare;
   uint16_t radiotimer_num_overflow;
   uint16_t radiotimer_num_compare;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== prototypes ======================================

void bsp_timer_cb_compare(void);
void radiotimer_cb_overflow(void);
void radiotimer_cb_compare(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {  
   // initialize board
   board_init();
   
   // switch radio LED on
   leds_radio_on();
   
   // prepare bsp_timer
   bsp_timer_set_callback(bsp_timer_cb_compare);
   
   // prepare radiotimer
   radiotimer_setOverflowCb(radiotimer_cb_overflow);
   radiotimer_setCompareCb(radiotimer_cb_compare);
   
   // kick off first bsp_timer compare
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
   // start periodic radiotimer overflow
   radiotimer_start(RADIOTIMER_OVERFLOW_PERIOD);
   
   // kick off first radiotimer compare
   app_vars.radiotimer_num_compares_left  = RADIOTIMER_NUM_COMPARES-1;
   app_vars.radiotimer_last_compare_val   = RADIOTIMER_COMPARE_PERIOD;
   radiotimer_schedule(app_vars.radiotimer_last_compare_val);
   
   while (1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void bsp_timer_cb_compare(void) {
   // toggle pin
   debugpins_frame_toggle();
   
   // toggle error led
   leds_sync_toggle();
   
   // increment counter
   app_dbg.bsp_timer_num_compare++;
   
   // schedule again
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
}

void radiotimer_cb_overflow(void) {
   volatile uint16_t delay;
   
   // toggle pin
   debugpins_slot_toggle();
   
   // switch radio LED on
   leds_error_toggle();
   
   // reset the counter for number of remaining compares
   app_vars.radiotimer_num_compares_left  = RADIOTIMER_NUM_COMPARES;
   app_vars.radiotimer_last_compare_val   = RADIOTIMER_COMPARE_PERIOD;
   radiotimer_schedule(app_vars.radiotimer_last_compare_val);
   
   // increment debug counter
   app_dbg.radiotimer_num_overflow++;
   
   // wait a bit
   for (delay=0;delay<ISR_DELAY;delay++);
}

void radiotimer_cb_compare(void) {
   // toggle pin
   debugpins_fsm_toggle();
   
   // toggle radio LED
   leds_radio_toggle();
   
   // schedule a next compare, if applicable
   app_vars.radiotimer_last_compare_val += RADIOTIMER_COMPARE_PERIOD;
   app_vars.radiotimer_num_compares_left--;
   if (app_vars.radiotimer_num_compares_left>0) {
      radiotimer_schedule(app_vars.radiotimer_last_compare_val);
   } else {
      radiotimer_cancel();
   }
   
   // increment debug counter
   app_dbg.radiotimer_num_compare++;
}