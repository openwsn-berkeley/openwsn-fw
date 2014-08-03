/**
\brief This program uses of both the "bsp_timer" and "radiotimer" bsp modules
       and schedules timers very close together, with long ISRs.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

This project is to verify all is well when the BSP timer and radiotimer are
fire at about the same time.

Load this program onto your board, and start running. It will enable the BSP
timer and radiotimer:
- the BSP timer is periodic, of period BSP_TIMER_PERIOD ticks. Each time it
  elapses:
    - the frame debugpin toggles
    - the error LED toggles
- the radiotimer overflows every RADIOTIMER_OVERFLOW_PERIOD. Each time it
  elapses:
    - the slot debugpin toggles
    - the radio LED toggles

Because the values of BSP_TIMER_PERIOD and RADIOTIMER_OVERFLOW_PERIOD are so
close, you should see the LEDs (and debugpins) slowly go out of phase and in
phase.

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
#define ISR_DELAY                      2000

//=========================== variables =======================================

typedef struct {
   uint16_t bsp_timer_num_compare;
   uint16_t radiotimer_num_overflow;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== prototypes ======================================

void bsp_timer_cb_compare(void);
void radiotimer_cb_overflow(void);
void radiotimer_cb_compare(void);
void small_delay(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {  
   
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

void bsp_timer_cb_compare(void) {
   
   // toggle pin
   debugpins_frame_toggle();
   
   // toggle error led
   leds_error_toggle();
   
   // increment counter
   app_dbg.bsp_timer_num_compare++;
   
   // schedule again
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
   // wait a bit
   small_delay();
}

void radiotimer_cb_overflow(void) {
   
   // toggle pin
   debugpins_slot_toggle();
   
   // switch radio LED on
   leds_radio_toggle();
   
   // increment counter
   app_dbg.radiotimer_num_overflow++;
   
   // wait a bit
   small_delay();
}

void radiotimer_cb_compare(void) {
   while(1);
}

void small_delay(void) {
   volatile uint16_t delay;
   for (delay=0;delay<ISR_DELAY;delay++);
}