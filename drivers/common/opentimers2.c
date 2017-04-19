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
void opentimers2_init(){
    // initialize local variables
    memset(&opentimers2_vars,0,sizeof(opentimers2_vars_t));
    // set callback for sctimer module
    sctimer_set_callback(opentimers2_timer_callback);
} 

/**
\brief create a timer by assigning an entry from timer buffer.

create a timer by reserving an Id for the timer.
 */
opentimers2_id_t opentimers2_create(void){
    uint8_t id;
    for (id=0;id<MAX_NUM_TIMERS;id++){
        if (opentimers2_vars.timersBuf[id].isUsed==FALSE){
            opentimers2_vars.timersBuf[id].isUsed = TRUE;
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
void opentimers2_scheduleRelative(opentimers2_id_t    id, 
                                  uint32_t            duration,
                                  time_type_t         uint_type, 
                                  opentimers2_cbt     cb){
    uint8_t  i;
    uint8_t idToSchedule;
    uint32_t durationTicks;
    uint32_t timerGap;
    // 1. make sure the timer exist
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isUsed && i == id){
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
    opentimers2_vars.timersBuf[id].isrunning           = TRUE;
    opentimers2_vars.timersBuf[id].callback            = cb;
    
    // 3. find the next timer to fire
    timerGap = MAX_TICKS_NUMBER;
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning){
            if (opentimers2_vars.timersBuf[i].currentCompareValue - opentimers2_vars.currentTimeout < timerGap){
                timerGap     = opentimers2_vars.timersBuf[i].currentCompareValue-opentimers2_vars.currentTimeout;
                idToSchedule = i;
            }
        }
    }
    
    // if I got here, assign the next to be fired timer to given timer
    opentimers2_vars.currentTimeout = opentimers2_vars.timersBuf[idToSchedule].currentCompareValue;
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
void opentimers2_scheduleAbsolute(opentimers2_id_t    id, 
                                  uint32_t            duration, 
                                  uint32_t            reference , 
                                  time_type_t         uint_type, 
                                  opentimers2_cbt     cb){
    uint8_t  i;
    uint8_t idToSchedule;
    uint32_t durationTicks;
    uint32_t timerGap;
    // 1. make sure the timer exist
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isUsed && i == id){
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
    opentimers2_vars.timersBuf[id].isrunning           = TRUE;
    opentimers2_vars.timersBuf[id].callback            = cb;
    
    // 3. find the next timer to fire
    timerGap = MAX_TICKS_NUMBER;
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning){
            if (opentimers2_vars.timersBuf[i].currentCompareValue - opentimers2_vars.lastTimeout < timerGap){
                timerGap     = opentimers2_vars.timersBuf[i].currentCompareValue-opentimers2_vars.lastTimeout;
                idToSchedule = i;
            }
        }
    }
    
    // if I got here, assign the next to be fired timer to given timer
    opentimers2_vars.currentTimeout = opentimers2_vars.timersBuf[idToSchedule].currentCompareValue;
    sctimer_setCompare(opentimers2_vars.currentTimeout);
    opentimers2_vars.running        = TRUE;
}

/**
\brief cancel a running timer.

This function disable the timer temperally by removing its callback and marking
isrunning as false. The timer may be recover later.

\param id the timer id
 */
void opentimers2_cancel(opentimers2_id_t id){
    opentimers2_vars.timersBuf[id].isrunning = FALSE;
    opentimers2_vars.timersBuf[id].callback  = NULL;
}

/**
\brief destroy a stored timer.

Reset the whole entry of given timer including the id.

\param id the timer id

\returns False if the given can't be found or return Success
 */
bool opentimers2_destroy(opentimers2_id_t id){
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
uint32_t opentimers2_getValue(opentimers2_id_t id){
    return sctimer_readCounter();
}

/**
\brief get the currentTimeout variable of opentimer2.

\returns currentTimeout.
 */
uint32_t opentimers2_getCurrentTimeout(){
    return opentimers2_vars.currentTimeout;
}

/**
\brief is the given timer running?

\returns isRunning variable.
 */
bool opentimers2_isRunning(opentimers2_id_t id){
    return opentimers2_vars.timersBuf[id].isrunning;
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
    uint8_t idToSchedule;
    uint32_t timerGap=MAX_TICKS_NUMBER;
    // 1. find the expired timer
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning==TRUE){
            if (opentimers2_vars.currentTimeout-opentimers2_vars.timersBuf[i].currentCompareValue < MAX_DURATION_ISR){
                // this timer expired, mark as expired
                opentimers2_vars.timersBuf[i].hasExpired = TRUE;
            }
        }
    }
    
    // 2. call the callback of expired timers
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].hasExpired == TRUE){
            opentimers2_vars.timersBuf[i].isrunning           = FALSE;
            opentimers2_vars.timersBuf[i].lastCompareValue    = opentimers2_vars.timersBuf[i].currentCompareValue;
            opentimers2_vars.timersBuf[i].hasExpired          = FALSE;
            opentimers2_vars.timersBuf[i].callback();
        }
    }
    // update lastTimeout
    opentimers2_vars.lastTimeout    = opentimers2_vars.currentTimeout;
      
    // 3. find the next timer to be fired
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers2_vars.timersBuf[i].isrunning==TRUE){
            if (opentimers2_vars.timersBuf[i].currentCompareValue-opentimers2_vars.lastTimeout<timerGap){
                timerGap     = opentimers2_vars.timersBuf[i].currentCompareValue-opentimers2_vars.lastTimeout;
                idToSchedule = i;
            }
        }
    }
    
    // 4. reschedule the timer
    opentimers2_vars.currentTimeout = opentimers2_vars.timersBuf[idToSchedule].currentCompareValue;
    sctimer_setCompare(opentimers2_vars.currentTimeout);
    opentimers2_vars.running        = TRUE;
}
