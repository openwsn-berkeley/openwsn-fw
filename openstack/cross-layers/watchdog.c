#include "opendefs.h"
#include "watchdog.h"
#include "ieee802154e.h"
#include "board.h"

//=========================== definition ======================================

#define WATCHDOG_COUNTER 1000  //ms

//=========================== variables =======================================

watchdog_vars_t watchdog_vars;

//=========================== prototypes ======================================

// timer interrupt callbacks
void watchdog_timer_cb(opentimer_id_t id);

//=========================== public ==========================================

void watchdog_init() {
    memset(&watchdog_vars,0,sizeof(watchdog_vars));
    watchdog_vars.bones = EBPERIOD+10;
    
    watchdog_vars.watchdogTimerId = opentimers_start(
       WATCHDOG_COUNTER,
       TIMER_PERIODIC,
       TIME_MS,
       watchdog_timer_cb
    );
}

//=========================== private =========================================

void watchdog_timer_cb(opentimer_id_t id){
    if (ieee154e_isSynch() == TRUE){
        watchdog_vars.bones = EBPERIOD+5;
        opentimers_stop(watchdog_vars.watchdogTimerId);
    } else {
        watchdog_vars.bones--;
        if (watchdog_vars.bones == 0){
            board_reset();
        }
    }
}