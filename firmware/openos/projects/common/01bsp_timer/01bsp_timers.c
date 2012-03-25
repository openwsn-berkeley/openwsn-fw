/**
\brief This program shows the use of the "timers" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "leds.h"
#include "bsp_timers.h"
#include "debugpins.h"
#include "radiotimer.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uint8_t radio_busy;
   uint8_t timer_busy;
   uint8_t num_overflow;
   uint8_t num_compare;
   uint8_t num_startFrame;
   uint8_t num_endFrame;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_timer_0();
void cb_timer_1();
void cb_timer_2();

//=========================== main ============================================
uint32_t count=0;
/**
\brief The program starts executing here.
*/
int main(void)
{  
   // initialize board
   board_init();
   
   radiotimer_setOverflowCb(cb_timer_0);
   radiotimer_setCompareCb(cb_timer_1);
   //radiotimer_setPeriod(327);
   radiotimer_start(327);
   radiotimer_schedule(33);

   // start timer_0
  /* timers_start(0,
                1,
                TIMER_PERIODIC,
                cb_timer_0);*/
   
//   // start timer_1
//   timers_start(1,
//                5000,
//                TIMER_PERIODIC,
//                cb_timer_1);
//
//   // start timer_2
//   timers_start(2,
//                7000,
//                TIMER_PERIODIC,
//                cb_timer_2);
//
   while (1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_timer_0() {
   //leds_error_toggle();
	debugpins_frame_toggle();

}

void cb_timer_1() {
  // leds_radio_toggle();
	debugpins_fsm_toggle();
	count++;
	if (count%2==0)	 radiotimer_schedule(66);
	else radiotimer_schedule(11);
}

void cb_timer_2() {
   leds_sync_toggle();
}
