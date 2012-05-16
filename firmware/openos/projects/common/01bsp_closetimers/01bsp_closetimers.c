/**
\brief This program uses of both the "bsp_timer" and "radiotimer" bsp modules
       and schedules timers very close together, with long ISRs.

This project is used to verify the correct behavior of the timer scheduling,
which is particularly important when using the sctimer timer abstractor.

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

#define BSP_TIMER_PERIOD               0x800
#define RADIOTIMER_OVERFLOW_PERIOD     0x801
#define ISR_DELAY                      1000

//=========================== variables =======================================

/*
typedef struct {
} app_vars_t;

app_vars_t app_vars;
*/

typedef struct {
   uint16_t bsp_timer_num_compare;
   uint16_t radiotimer_num_overflow;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== prototypes ======================================

void bsp_timer_cb_compare();
void radiotimer_cb_overflow();
void radiotimer_cb_compare();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void)
{  
   // initialize board
   board_init();
   
   // prepare bsp_timer
   bsp_timer_set_callback(bsp_timer_cb_compare);
   
   // prepare radiotimer
   radiotimer_setOverflowCb(radiotimer_cb_overflow);
   radiotimer_setCompareCb(radiotimer_cb_compare);
   
   // kick off first bsp_timer compare
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
   // start periodic radiotimer overflow
   radiotimer_start(RADIOTIMER_OVERFLOW_PERIOD);
   
   while (1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void bsp_timer_cb_compare() {
   uint16_t delay;
   
   // toggle pin
   debugpins_fsm_toggle();
   
   // toggle error led
   leds_error_toggle();
   
   // increment counter
   app_dbg.bsp_timer_num_compare++;
   
   // schedule again
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
   // wait a bit
   for (delay=0;delay<ISR_DELAY;delay++);
}

void radiotimer_cb_overflow() {
   uint16_t delay;
   
   // toggle pin
   debugpins_frame_toggle();
   
   // switch radio LED on
   leds_radio_toggle();
   
   // increment debug counter
   app_dbg.radiotimer_num_overflow++;
   
   // wait a bit
   for (delay=0;delay<ISR_DELAY;delay++);
}

void radiotimer_cb_compare() {
   while(1);
}