/**
\brief Blink two LEDs using two different tasks

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, October 2014.
*/

#include "board.h"
#include "scheduler.h"
#include "opentimers.h"
#include "leds.h"

//=========================== defines ==========================================

//=========================== structs =========================================

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  errorLedTimerId;
   opentimer_id_t  radioLedTimerId;
} blinky_vars_t;

blinky_vars_t blinky_vars;

//=========================== prototypes ======================================

void cb_errorLedTimer(void);
void task_blink_errorLed(void);
void cb_radioLedTimer(void);
void task_blink_radioLed(void);

//=========================== initialization ==================================

int mote_main(void) {
   
   board_init();
   scheduler_init();
   opentimers_init();
   
   blinky_vars.errorLedTimerId  = opentimers_start(
      50,
      TIMER_PERIODIC,
      TIME_MS,
      cb_errorLedTimer
   );
   
   blinky_vars.radioLedTimerId  = opentimers_start(
      500,
      TIMER_PERIODIC,
      TIME_MS,
      cb_radioLedTimer
   );
   
   scheduler_start();
   return 0; // this line should never be reached
}


void vApplicationIdleHook( void ) {
	board_sleep();
}

//=========================== private =========================================

void cb_errorLedTimer(void) {
   scheduler_push_task(task_blink_errorLed,TASKPRIO_SIXTOP);
}

void task_blink_errorLed(void) {
   leds_error_toggle();
}

void cb_radioLedTimer(void) {
   scheduler_push_task(task_blink_radioLed,TASKPRIO_STACK_TRANSPORT);
}

void task_blink_radioLed(void) {
   //while(1) {
      leds_radio_toggle();
   //}
}
