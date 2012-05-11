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

//=========================== defines =========================================

#define BSP_TIMER_PERIOD     0x7fff // @32kHz = 1s

//=========================== variables =======================================

typedef struct {
   uint16_t num_compare;
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
   
   bsp_timer_set_callback(cb_compare);
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
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
   
   // schedule again
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
}