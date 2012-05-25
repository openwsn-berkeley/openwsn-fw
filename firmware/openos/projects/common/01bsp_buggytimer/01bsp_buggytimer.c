/**
\brief This program shows the use of the "bsp_buggytimer" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "bsp_timer.h"

//=========================== defines =========================================

#define BSP_TIMER_PERIOD     0x7fff // @32kHz = 1s
#define RADIO_TIMER_PERIOD    500
#define RADIO_TIMER_COMPARE   125

//=========================== variables =======================================

typedef struct {
   uint16_t num_bsp_compare;
   uint16_t num_radio_overflow;
   uint16_t num_radio_compare;
   uint16_t next_radio_timer_compare;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_bsp_compare();
void cb_radiotimer_period();
void cb_radiotimer_compare();
//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void)
{  
   // initialize board
   board_init();
   
   bsp_timer_set_callback(cb_bsp_compare);
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
   radiotimer_setCompareCb(cb_radiotimer_compare);
   radiotimer_setOverflowCb(cb_radiotimer_period);
   
   radiotimer_start(RADIO_TIMER_PERIOD);
   
   app_vars.next_radio_timer_compare=RADIO_TIMER_COMPARE;
   radiotimer_schedule(app_vars.next_radio_timer_compare);
   
   while (1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_bsp_compare() {
   // toggle pin
   debugpins_fsm_toggle();
   
   // toggle error led
   leds_error_toggle();
   
   // increment counter
   app_vars.num_bsp_compare++;
   
   // schedule again
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
}


void cb_radiotimer_period() {
   // toggle pin
   debugpins_slot_toggle();
   
   // toggle error led
   leds_error_toggle();
   
   // increment counter
   app_vars.num_radio_overflow++;
   
   // schedule again
   radiotimer_setPeriod(RADIO_TIMER_PERIOD);
}



void cb_radiotimer_compare() {
   // toggle pin
   debugpins_isr_toggle();
   
   app_vars.next_radio_timer_compare+=RADIO_TIMER_COMPARE;
   app_vars.next_radio_timer_compare=app_vars.next_radio_timer_compare%RADIO_TIMER_PERIOD;
   // toggle error led
   leds_error_toggle();
   
   // increment counter
   app_vars.num_radio_compare++;
   
   // schedule again
   radiotimer_schedule(app_vars.next_radio_timer_compare);
}

