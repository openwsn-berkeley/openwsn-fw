/**
\brief Mercator firmware, see https://github.com/openwsn-berkeley/mercator/.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

// stack initialization
#include "board.h"
#include "scheduler.h"
#include "opentimers.h"
#include "openserial.h"
#include "leds.h"

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
} mercator_vars_t;

mercator_vars_t mercator_vars;

//=========================== prototypes ======================================

void mercator_toggle_led(void);

//=========================== initialization ==================================

int mote_main(void) {
   board_init();
   scheduler_init();
   opentimers_init();
   mercator_vars.timerId  = opentimers_start(
      500,
      TIMER_PERIODIC,TIME_MS,
      mercator_toggle_led
   );
   scheduler_start();
   return 0; // this line should never be reached
}

void mercator_toggle_led(void) {
   leds_error_toggle();
}
