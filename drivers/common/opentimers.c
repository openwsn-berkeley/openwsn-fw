/**
\brief Definition of the "opentimers" driver.

This driver uses a single hardware timer, which it virtualizes to support
at most MAX_NUM_TIMERS timers.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
 */

#include "opendefs.h"
#include "opentimers.h"
#include "sctimer.h"
#include "leds.h"

//=========================== define ==========================================

//=========================== variables =======================================

opentimers_vars_t opentimers_vars;

//=========================== prototypes ======================================

void opentimers_timer_callback(void);

//=========================== public ==========================================

/**
\brief Initialize this module.

Initializes data structures and hardware timer.
 */
void opentimers_init(void){
    uint8_t i;
    // initialize local variables
    memset(&opentimers_vars,0,sizeof(opentimers_vars_t));
    for (i=0;i<MAX_NUM_TIMERS;i++){
        // by default, all timers have the priority of 0xff (lowest priority)
        opentimers_vars.timersBuf[i].priority = 0xff;
    }
    // set callback for sctimer module
    sctimer_set_callback(opentimers_timer_callback);
} 

/**
\brief create a timer by assigning an entry from timer buffer.

create a timer by assigning an Id for the timer.

\returns the id of the timer will be returned
 */
opentimers_id_t opentimers_create(void){
    uint8_t id;
    for (id=0;id<MAX_NUM_TIMERS;id++){
        if (opentimers_vars.timersBuf[id].isUsed  == FALSE){
            opentimers_vars.timersBuf[id].isUsed   = TRUE;
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

\param[in] id indicates the timer id
\param[in] duration indicates the period asked for schedule since last comparing value
\param[in] uint_type indicates the unit type of this schedule: ticks or ms
\param[in] cb indicates when this scheduled timer fired, call this callback function.
 */
void opentimers_scheduleRelative(opentimers_id_t    id, 
                                  uint32_t            duration,
                                  time_type_t         uint_type, 
                                  opentimers_cbt     cb){
    uint8_t  i;
    uint8_t idToSchedule;
    uint32_t durationTicks;
    uint32_t timerGap;
    uint32_t tempTimerGap;
    // 1. make sure the timer exist
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers_vars.timersBuf[i].isUsed && i == id){
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
    
    opentimers_vars.timersBuf[id].currentCompareValue = durationTicks+opentimers_vars.timersBuf[id].lastCompareValue;
    opentimers_vars.timersBuf[id].isrunning           = TRUE;
    opentimers_vars.timersBuf[id].callback            = cb;
    
    // 3. find the next timer to fire
    timerGap     = opentimers_vars.timersBuf[0].currentCompareValue-opentimers_vars.lastTimeout;
    idToSchedule = 0;
    for (i=1;i<MAX_NUM_TIMERS;i++){
        if (opentimers_vars.timersBuf[i].isrunning){
            tempTimerGap = opentimers_vars.timersBuf[i].currentCompareValue - opentimers_vars.lastTimeout;
            if (tempTimerGap < timerGap){
                // if a timer "i" has low priority but has compare value less than 
                // candidate timer "idToSchedule" more than TIMERTHRESHOLD ticks, 
                // replace candidate timer by this timer "i".
                if (opentimers_vars.timersBuf[i].priority > opentimers_vars.timersBuf[idToSchedule].priority){
                    if (timerGap-tempTimerGap > TIMERTHRESHOLD){
                        timerGap     = tempTimerGap;
                        idToSchedule = i;
                    }
                } else {
                    // a timer "i" has higher priority than candidate timer "idToSchedule" 
                    // and compare value less than candidate timer replace candidate 
                    // timer by timer "i".
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            } else {
                // if a timer "i" has higher priority than candidate timer "idToSchedule" 
                // and its compare value is larger than timer "i" no more than TIMERTHRESHOLD ticks,
                // replace candidate timer by timer "i".
                if (opentimers_vars.timersBuf[i].priority < opentimers_vars.timersBuf[idToSchedule].priority){
                    if (tempTimerGap - timerGap < TIMERTHRESHOLD){
                        timerGap     = tempTimerGap;
                        idToSchedule = i;
                    }
                }
            }
        }
    }
    
    // if I got here, assign the next to be fired timer to given timer
    opentimers_vars.currentTimeout = opentimers_vars.timersBuf[idToSchedule].currentCompareValue;
    sctimer_setCompare(opentimers_vars.currentTimeout);
    opentimers_vars.running        = TRUE;
}

/**
\brief schedule a period refer to given reference.

This function will schedule a timer which expires when the timer count reach 
to lastCompareValue + reference.

\param[in] id indicates the timer id
\param[in] duration indicates the period asked for schedule after a given time indicated by reference parameter.
\param[in] uint_type indicates the unit type of this schedule: ticks or ms
\param[in] cb indicates when this scheduled timer fired, call this callback function.
 */
void opentimers_scheduleAbsolute(opentimers_id_t    id, 
                                  uint32_t            duration, 
                                  uint32_t            reference , 
                                  time_type_t         uint_type, 
                                  opentimers_cbt     cb){
    uint8_t  i;
    uint8_t idToSchedule;
    uint32_t durationTicks;
    uint32_t timerGap;
    uint32_t tempTimerGap;
    // 1. make sure the timer exist
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers_vars.timersBuf[i].isUsed && i == id){
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
    
    opentimers_vars.timersBuf[id].currentCompareValue = durationTicks+reference;
    opentimers_vars.timersBuf[id].isrunning           = TRUE;
    opentimers_vars.timersBuf[id].callback            = cb;
    
    // 3. find the next timer to fire
    timerGap     = opentimers_vars.timersBuf[0].currentCompareValue-opentimers_vars.lastTimeout;
    idToSchedule = 0;
    for (i=1;i<MAX_NUM_TIMERS;i++){
        if (opentimers_vars.timersBuf[i].isrunning){
            tempTimerGap = opentimers_vars.timersBuf[i].currentCompareValue - opentimers_vars.lastTimeout;
            if (tempTimerGap < timerGap){
                // if a timer "i" has low priority but has compare value less than 
                // candidate timer "idToSchedule" more than TIMERTHRESHOLD ticks, 
                // replace candidate timer by this timer "i".
                if (opentimers_vars.timersBuf[i].priority > opentimers_vars.timersBuf[idToSchedule].priority){
                    if (timerGap-tempTimerGap > TIMERTHRESHOLD){
                        timerGap     = tempTimerGap;
                        idToSchedule = i;
                    }
                } else {
                    // a timer "i" has higher priority than candidate timer "idToSchedule" 
                    // and compare value less than candidate timer replace candidate 
                    // timer by timer "i".
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            } else {
                // if a timer "i" has higher priority than candidate timer "idToSchedule" 
                // and its compare value is larger than timer "i" no more than TIMERTHRESHOLD ticks,
                // replace candidate timer by timer "i".
                if (opentimers_vars.timersBuf[i].priority < opentimers_vars.timersBuf[idToSchedule].priority){
                    if (tempTimerGap - timerGap < TIMERTHRESHOLD){
                        timerGap     = tempTimerGap;
                        idToSchedule = i;
                    }
                }
            }
        }
    }
    
    // if I got here, assign the next to be fired timer to given timer
    opentimers_vars.currentTimeout = opentimers_vars.timersBuf[idToSchedule].currentCompareValue;
    sctimer_setCompare(opentimers_vars.currentTimeout);
    opentimers_vars.running        = TRUE;
}

/**
\brief cancel a running timer.

This function disable the timer temperally by removing its callback and marking
isrunning as false. The timer may be recover later.

\param[in] id the timer id
 */
void opentimers_cancel(opentimers_id_t id){
    opentimers_vars.timersBuf[id].isrunning = FALSE;
    opentimers_vars.timersBuf[id].callback  = NULL;
}

/**
\brief destroy a stored timer.

Reset the whole entry of given timer including the id.

\param[in] id the timer id

\returns False if the given can't be found or return Success
 */
bool opentimers_destroy(opentimers_id_t id){
    if (id<MAX_NUM_TIMERS){
        memset(&opentimers_vars.timersBuf[id],0,sizeof(opentimers_t));
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
\brief get the counter value of given timer.

\param[in] id the timer id

\returns the current counter value.
 */
uint32_t opentimers_getValue(opentimers_id_t id){
    return sctimer_readCounter();
}

/**
\brief get the currentTimeout variable of opentimer2.

\returns currentTimeout.
 */
uint32_t opentimers_getCurrentTimeout(void){
    return opentimers_vars.currentTimeout;
}

/**
\brief is the given timer running?

\returns isRunning variable.
 */
bool opentimers_isRunning(opentimers_id_t id){
    return opentimers_vars.timersBuf[id].isrunning;
}


/**
\brief set the priority of given timer

\param[in] id indicates the timer to be assigned.
\param[in] priority indicates the priority of given timer.
 */
void opentimers_setPriority(opentimers_id_t id, uint8_t priority){
    if (opentimers_vars.timersBuf[id].isUsed  == TRUE){
        opentimers_vars.timersBuf[id].priority = priority;
    } else {
        // the given timer is not used, do nothing.
    }
}

// ========================== callback ========================================

/**
\brief this is the callback function of opentimer2.

This function is called when sctimer interrupt happens. The function looks the 
whole timer buffer and find out the correct timer responding to the interrupt
and call the callback recorded for that timer.
 */
void opentimers_timer_callback(void){
    uint8_t i;
    uint8_t j;
    uint8_t idToCallCB;
    uint8_t idToSchedule;
    uint32_t timerGap;
    uint32_t tempTimerGap;
    uint32_t tempLastTimeout = opentimers_vars.currentTimeout;
    // 1. find the expired timer
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers_vars.timersBuf[i].isrunning==TRUE){
            // all timers in the past within TIMERTHRESHOLD ticks
            // (probably with low priority) will mared as Expired.
            if (opentimers_vars.currentTimeout-opentimers_vars.timersBuf[i].currentCompareValue <= TIMERTHRESHOLD){
                // this timer expired, mark as expired
                opentimers_vars.timersBuf[i].hasExpired = TRUE;
                // find the fired timer who has the smallest currentTimeout as last Timeout
                if (tempLastTimeout>opentimers_vars.timersBuf[i].currentCompareValue){
                    tempLastTimeout = opentimers_vars.timersBuf[i].currentCompareValue;
                }
            }
        }
    }
    
    // update lastTimeout
    opentimers_vars.lastTimeout                               = tempLastTimeout;
    
    // 2. call the callback of expired timers
    idToCallCB = TOO_MANY_TIMERS_ERROR;
    // find out the timer expired with highest priority 
    for (j=0;j<MAX_NUM_TIMERS;j++){
        if (opentimers_vars.timersBuf[j].hasExpired == TRUE){
            if (idToCallCB==TOO_MANY_TIMERS_ERROR){
                idToCallCB = j;
            } else {
                if (opentimers_vars.timersBuf[j].priority<opentimers_vars.timersBuf[idToCallCB].priority){
                    idToCallCB = j;
                }
            }
        }
    }
    if (idToCallCB==TOO_MANY_TIMERS_ERROR){
        // no more timer expired
    } else {
        // call all timers expired having the same priority with timer idToCallCB
        for (j=0;j<MAX_NUM_TIMERS;j++){
            if (
                opentimers_vars.timersBuf[j].hasExpired == TRUE &&
                opentimers_vars.timersBuf[j].priority   == opentimers_vars.timersBuf[idToCallCB].priority
            ){
                opentimers_vars.timersBuf[j].isrunning           = FALSE;
                opentimers_vars.timersBuf[j].lastCompareValue    = opentimers_vars.timersBuf[j].currentCompareValue;
                opentimers_vars.timersBuf[j].hasExpired          = FALSE;
                opentimers_vars.timersBuf[j].callback();
            }
        }
    }
      
    // 3. find the next timer to be fired
    timerGap     = opentimers_vars.timersBuf[0].currentCompareValue-opentimers_vars.lastTimeout;
    idToSchedule = 0;
    for (i=1;i<MAX_NUM_TIMERS;i++){
        if (opentimers_vars.timersBuf[i].isrunning){
            tempTimerGap = opentimers_vars.timersBuf[i].currentCompareValue - opentimers_vars.lastTimeout;
            if (tempTimerGap < timerGap){
                // if a timer "i" has low priority but has compare value less than 
                // candidate timer "idToSchedule" more than TIMERTHRESHOLD ticks, 
                // replace candidate timer by this timer "i".
                if (opentimers_vars.timersBuf[i].priority > opentimers_vars.timersBuf[idToSchedule].priority){
                    if (timerGap-tempTimerGap > TIMERTHRESHOLD){
                        timerGap     = tempTimerGap;
                        idToSchedule = i;
                    }
                } else {
                    // a timer "i" has higher priority than candidate timer "idToSchedule" 
                    // and compare value less than candidate timer replace candidate 
                    // timer by timer "i".
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            } else {
                // if a timer "i" has higher priority than candidate timer "idToSchedule" 
                // and its compare value is larger than timer "i" no more than TIMERTHRESHOLD ticks,
                // replace candidate timer by timer "i".
                if (opentimers_vars.timersBuf[i].priority < opentimers_vars.timersBuf[idToSchedule].priority){
                    if (tempTimerGap - timerGap < TIMERTHRESHOLD){
                        timerGap     = tempTimerGap;
                        idToSchedule = i;
                    }
                }
            }
        }
    }
    
    // 4. reschedule the timer
    opentimers_vars.currentTimeout = opentimers_vars.timersBuf[idToSchedule].currentCompareValue;
    sctimer_setCompare(opentimers_vars.currentTimeout);
    opentimers_vars.running        = TRUE;
}






















































