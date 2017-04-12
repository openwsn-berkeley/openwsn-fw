/**
\brief Definition of the "opentimers2" driver.

This driver uses a single hardware timer, which it virtualizes to support
at most MAX_NUM_TIMERS timers.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
 */

#include "opendefs.h"
#include "opentimers2.h"
#include "sctimer.h"
#include "leds.h"

//=========================== define ==========================================

//=========================== variables =======================================

opentimers2_vars_t opentimers2_vars;

//=========================== prototypes ======================================

void opentimers2_timer_callback(void);

//=========================== public ==========================================

/**
\brief Initialize this module.

Initializes data structures and hardware timer.
 */
void opentimer2_init(){
    // initialize local variables
    memset(&opentimers2_vars,0,sizeof(opentimers2_vars_t));
    // set callback for sctimer module
    sctimer_set_callback(opentimers2_timer_callback);
} 

/**
\brief create a timer by assigning an entry from timer buffer.

create a timer by reserving an Id for the timer.
 */
opentimer2_id_t opentimer2_create(void){
    uint8_t id;
    for (id=0;id<MAX_NUM_TIMERS;id++){
        if (opentimers2_vars.timersBuf[id].isrunning==FALSE){
            return id;
        }
    }
    // there is no available buffer for this timer
    return TOO_MANY_TIMERS_ERROR;
}

/**
\brief schedule a period refer to comparing value set last time.

This function will schedule a timer which expires when the timer count reach 
to lastCompareValue + duration.

Note: as this function schedule time depending on last compare value. It 
can't be called firstly after the timer is created.

\param id the timer id
\param duration the period asked for schedule since last comparing value
\param uint_type the unit type of this schedule: ticks or ms
\param cb when this scheduled timer fired, call this callback function.
 */
void opentimer2_scheduleRelative(opentimer2_id_t     id, 
                                 uint32_t            duration,
                                 uint_type_t         uint_type, 
                                 opentimers2_cbt     cb){
    uint8_t  i;
    uint32_t durationTicks;
    uint32_t timerGap;
    // 1. make sure the timer exist
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning && i == id){
            break;
        }
    }
    if (i==MAX_NUM_TIMERS){
        // doesn't find the timer
        return;
    }
    
    // 2. updat the timer content
    switch (uint_type){
    case TIME_MS:
        if (duration>MAX_TICKS_NUMBER/PORT_TICS_PER_MS){
            // openserail_printError();
            return;
        } else {
            durationTicks = duration*PORT_TICS_PER_MS;
        }
        break;
    case TIME_TICS:
        durationTicks = duration;
        break;
    }
    
    opentimers2_vars.timersBuf[id].currentCompareValue = durationTicks+opentimers2_vars.timersBuf[id].lastCompareValue;
    
    // 3. update the next timer to fire
    timerGap = opentimers2_vars.timersBuf[id].currentCompareValue-opentimers2_vars.currentTimeout;
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning){
            if (opentimers2_vars.timersBuf[i].currentCompareValue - opentimers2_vars.currentTimeout < timerGap){
                // there is one timer will fired before the given timer, needn't update
                return;
            }
        }
    }
    // if I got here, assign the next to be fired timer to given timer
    opentimers2_vars.currentTimeout = opentimers2_vars.timersBuf[id].currentCompareValue;
    sctimer_setCompare(opentimers2_vars.currentTimeout);
    opentimers2_vars.running        = TRUE;
}

/**
\brief schedule a period refer to given reference.

This function will schedule a timer which expires when the timer count reach 
to lastCompareValue + reference.

\param id the timer id
\param duration the period asked for schedule after a given time indicated by reference parameter.
\param uint_type the unit type of this schedule: ticks or ms
\param cb when this scheduled timer fired, call this callback function.
 */
void opentimer2_scheduleAbsolute(opentimer2_id_t     id, 
                                 uint32_t            duration, 
                                 uint32_t            reference , 
                                 uint_type_t         uint_type, 
                                 opentimers2_cbt     cb){
    uint8_t  i;
    uint32_t durationTicks;
    uint32_t timerGap;
    // 1. make sure the timer exist
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning && i == id){
            break;
        }
    }
    if (i==MAX_NUM_TIMERS){
        // doesn't find the timer
        return;
    }
    
    // 2. updat the timer content
    switch (uint_type){
    case TIME_MS:
        if (duration>MAX_TICKS_NUMBER/PORT_TICS_PER_MS){
            // openserail_printError("unsupported large duration");
            return;
        } else {
            durationTicks = duration*PORT_TICS_PER_MS;
        }
        break;
    case TIME_TICS:
        durationTicks = duration;
        break;
    }
    
    opentimers2_vars.timersBuf[id].currentCompareValue = durationTicks+reference;
    
    // 3. update the next timer to fire
    timerGap = opentimers2_vars.timersBuf[id].currentCompareValue-opentimers2_vars.currentTimeout;
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning){
            if (opentimers2_vars.timersBuf[i].currentCompareValue - opentimers2_vars.currentTimeout < timerGap){
                // there is one timer will fired before the given timer, needn't update
                return;
            }
        }
    }
    // if I got here, assign the next to be fired timer to given timer
    opentimers2_vars.currentTimeout = opentimers2_vars.timersBuf[id].currentCompareValue;
    sctimer_setCompare(opentimers2_vars.currentTimeout);
    opentimers2_vars.running        = TRUE;
}

/**
\brief cancel a running timer.

This function disable the timer temperally by removing its callback and marking
isrunning as false. The timer may be recover later.

\param id the timer id
 */
void opentimer2_cancel(opentimer2_id_t id){
    opentimers2_vars.timersBuf[id].isrunning = FALSE;
    opentimers2_vars.timersBuf[id].callback  = NULL;
}

/**
\brief destroy a stored timer.

Reset the whole entry of given timer including the id.

\param id the timer id

\returns False if the given can't be found or return Success
 */
bool opentimer2_destroy(opentimer2_id_t id){
    if (id<MAX_NUM_TIMERS){
        memset(&opentimers2_vars.timersBuf[id],0,sizeof(opentimers2_t));
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
\brief get the counter value of given timer.

\param id the timer id

\returns the current counter value.
 */
uint32_t opentimer2_getValue(opentimer2_id_t id){
    return sctimer_readCounter();
}

// ========================== callback ========================================

/**
\brief this is the callback function of opentimer2.

This function is called when sctimer interrupt happens. The function looks the 
whole timer buffer and find out the correct timer responding to the interrupt
and call the callback recorded for that timer.
 */
void opentimers2_timer_callback(void){
    uint8_t i;
    uint8_t id;
    uint32_t timerGap=MAX_TICKS_NUMBER;
    // 1. find the expired timer
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning==TRUE){
            if (opentimers2_vars.currentTimeout-opentimers2_vars.timersBuf[i].currentCompareValue < MAX_DURATION_ISR){
                // this timer expired, break
                id = i;
                break;
            }
        }
    }
    
    // 2. call the callback of expired timer
    if (i==MAX_NUM_TIMERS){
        // openserail_printError("failed to find expired timer")
        return;
    }
    opentimers2_vars.timersBuf[id].callback(id);
    opentimers2_vars.timersBuf[id].isrunning = FALSE;
    
      
    // 3. find the next timer to be fired
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning==TRUE){
            if (opentimers2_vars.timersBuf[i].currentCompareValue-opentimers2_vars.currentTimeout<timerGap){
                timerGap = opentimers2_vars.timersBuf[i].currentCompareValue-opentimers2_vars.currentTimeout;
                id = i;
            }
        }
    }
    
    // 4. reschedule the timer
    opentimers2_vars.currentTimeout = opentimers2_vars.timersBuf[id].currentCompareValue;
    sctimer_setCompare(opentimers2_vars.currentTimeout);
    opentimers2_vars.running        = TRUE;
}
