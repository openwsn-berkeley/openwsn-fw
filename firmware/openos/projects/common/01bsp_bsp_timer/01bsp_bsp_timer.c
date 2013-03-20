/**
\brief This program shows the use of the "bsp_timer" bsp module.

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
#include "flextimer.h"
#include "opentimers.h"

//=========================== defines =========================================

#define BSP_TIMER_PERIOD     281 // @32kHz = 1s

//=========================== variables =======================================

typedef struct {
   uint16_t num_compare;
   uint16_t prev;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_compare();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void)
{  
   // initialize board
   board_init();
//   flextimer_init();
//   flextimer_setCb(cb_compare);
//   app_vars.prev=0;
//   app_vars.num_compare=0;
//    
//   
//   flextimer_schedule(BSP_TIMER_PERIOD);
   app_vars.prev=BSP_TIMER_PERIOD;
   
  // bsp_timer_set_callback(cb_compare);
  // bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   opentimers_init();
   opentimers_start(1000,TIMER_ONESHOT,TIME_MS,cb_compare);
   while (1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_compare() {
   // toggle pin
   debugpins_fsm_toggle();
   
   // toggle error led
   leds_error_toggle();
   
   // increment counter
   app_vars.num_compare++;
   app_vars.prev+=BSP_TIMER_PERIOD;
   // schedule again
  // bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
  // flextimer_schedule(app_vars.prev);
   opentimers_start(3000,TIMER_ONESHOT,TIME_MS,cb_compare);
}