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
#include "scheduler.h"

//=========================== define ==========================================

//=========================== variables =======================================

opentimers_vars_t opentimers_vars;

//=========================== prototypes ======================================

void  opentimers_timer_callback(void);

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

create a timer by assigning an Id and priority for the timer

\returns the id of the timer will be returned
 */
opentimers_id_t opentimers_create(uint8_t priority){
    uint8_t id;

    if (priority==0){
        if (opentimers_vars.timersBuf[0].isUsed  == FALSE){
            opentimers_vars.timersBuf[0].isUsed   = TRUE;
            opentimers_vars.timersBuf[0].priority = 0;
            return 0;
        }
    } else {
        for (id=1;id<MAX_NUM_TIMERS;id++){
            if (opentimers_vars.timersBuf[id].isUsed  == FALSE){
                opentimers_vars.timersBuf[id].isUsed   = TRUE;
                opentimers_vars.timersBuf[id].priority  = priority;
                return id;
            }
        }
    }
    // there is no available buffer for this timer
    return ERROR_NO_AVAILABLE_ENTRIES;
}

/**
\brief schedule a period refer to comparing value set last time.

This function will schedule a timer which expires when the timer count reach
to current counter + duration.

\param[in] id indicates the timer id
\param[in] duration indicates the period asked for schedule since last comparing value
\param[in] uint_type indicates the unit type of this schedule: ticks or ms
\param[in] timer_type indicates the timer type of this schedule: oneshot or periodic
\param[in] cb indicates when this scheduled timer fired, call this callback function.
 */
void opentimers_scheduleIn(opentimers_id_t    id,
                           uint32_t           duration,
                           time_type_t        uint_type,
                           timer_type_t       timer_type,
                           opentimers_cbt     cb){
    uint8_t  i;
    uint8_t  idToSchedule;
    PORT_TIMER_WIDTH timerGap;
    PORT_TIMER_WIDTH tempTimerGap;
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

    opentimers_vars.timersBuf[id].timerType = timer_type;

    // 2. updat the timer content
    switch (uint_type){
    case TIME_MS:
        opentimers_vars.timersBuf[id].duration = duration*PORT_TICS_PER_MS;
        opentimers_vars.timersBuf[id].wraps_remaining  = (uint32_t)(duration*PORT_TICS_PER_MS)/MAX_TICKS_IN_SINGLE_CLOCK;
        break;
    case TIME_TICS:
        opentimers_vars.timersBuf[id].duration = duration;
        opentimers_vars.timersBuf[id].wraps_remaining  = (uint32_t)(duration)/MAX_TICKS_IN_SINGLE_CLOCK;
        break;
    }

    if (opentimers_vars.timersBuf[id].wraps_remaining==0){
        opentimers_vars.timersBuf[id].currentCompareValue = opentimers_vars.timersBuf[id].duration+sctimer_readCounter();
    } else {
        opentimers_vars.timersBuf[id].currentCompareValue = MAX_TICKS_IN_SINGLE_CLOCK+sctimer_readCounter();
    }

    opentimers_vars.timersBuf[id].isrunning           = TRUE;
    opentimers_vars.timersBuf[id].callback            = cb;

    // 3. find the next timer to fire

    // only execute update the currentCompareValue if I am not inside of ISR or the ISR itself will do this.
    if (opentimers_vars.insideISR==FALSE){
        i = 0;
        while (opentimers_vars.timersBuf[i].isrunning==FALSE){
            i++;
        }
        timerGap     = opentimers_vars.timersBuf[i].currentCompareValue-opentimers_vars.lastCompareValue;
        idToSchedule = i;
        for (i=idToSchedule+1;i<MAX_NUM_TIMERS;i++){
            if (opentimers_vars.timersBuf[i].isrunning){
                tempTimerGap = opentimers_vars.timersBuf[i].currentCompareValue-opentimers_vars.lastCompareValue;
                if (tempTimerGap < timerGap){
                    // timer "i" is more close to lastCompare value
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            }
        }

        // if I got here, assign the next to be fired timer to given timer
        opentimers_vars.currentCompareValue = opentimers_vars.timersBuf[idToSchedule].currentCompareValue;
        sctimer_setCompare(opentimers_vars.currentCompareValue);
    }
    opentimers_vars.running        = TRUE;
}

/**
\brief schedule a period refer to given reference.

This function will schedule a timer which expires when the timer count reach
to duration + reference. This function will be used in the implementation of slot FSM.
All timers use this function are ONE_SHOT type timer.

\param[in] id indicates the timer id
\param[in] duration indicates the period asked for schedule after a given time indicated by reference parameter.
\param[in] reference indicates the reference for duration. The timer will be fired at reference+duration.
\param[in] uint_type indicates the unit type of this schedule: ticks or ms
\param[in] cb indicates when this scheduled timer fired, call this callback function.
 */
void opentimers_scheduleAbsolute(opentimers_id_t    id,
                                 uint32_t           duration,
                                 PORT_TIMER_WIDTH   reference ,
                                 time_type_t        uint_type,
                                 opentimers_cbt     cb){
    uint8_t  i;
    uint8_t idToSchedule;
    PORT_TIMER_WIDTH timerGap;
    PORT_TIMER_WIDTH tempTimerGap;

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

    // absolute scheduling is for one shot timer
    opentimers_vars.timersBuf[id].timerType = TIMER_ONESHOT;

    // 2. updat the timer content
    switch (uint_type){
    case TIME_MS:
        opentimers_vars.timersBuf[id].duration = duration*PORT_TICS_PER_MS;
        opentimers_vars.timersBuf[id].wraps_remaining  = (uint32_t)(duration*PORT_TICS_PER_MS)/MAX_TICKS_IN_SINGLE_CLOCK;;
        break;
    case TIME_TICS:
        opentimers_vars.timersBuf[id].duration = duration;
        opentimers_vars.timersBuf[id].wraps_remaining  = (uint32_t)duration/MAX_TICKS_IN_SINGLE_CLOCK;
        break;
    }

    if (opentimers_vars.timersBuf[id].wraps_remaining==0){
        opentimers_vars.timersBuf[id].currentCompareValue = opentimers_vars.timersBuf[id].duration+reference;
    } else {
        opentimers_vars.timersBuf[id].currentCompareValue = MAX_TICKS_IN_SINGLE_CLOCK+reference;
    }

    opentimers_vars.timersBuf[id].isrunning = TRUE;
    opentimers_vars.timersBuf[id].callback  = cb;

    // 3. find the next timer to fire

    // only execute update the currentCompareValue if I am not inside of ISR or the ISR itself will do this.
    if (opentimers_vars.insideISR==FALSE){
        i = 0;
        while (opentimers_vars.timersBuf[i].isrunning==FALSE){
            i++;
        }
        timerGap     = opentimers_vars.timersBuf[i].currentCompareValue-opentimers_vars.lastCompareValue;
        idToSchedule = i;
        for (i=idToSchedule+1;i<MAX_NUM_TIMERS;i++){
            if (opentimers_vars.timersBuf[i].isrunning){
                tempTimerGap = opentimers_vars.timersBuf[i].currentCompareValue-opentimers_vars.lastCompareValue;
                if (tempTimerGap < timerGap){
                    // timer "i" is more close to lastCompare value
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            }
        }

        // if I got here, assign the next to be fired timer to given timer
        opentimers_vars.currentCompareValue = opentimers_vars.timersBuf[idToSchedule].currentCompareValue;
        sctimer_setCompare(opentimers_vars.currentCompareValue);
    }
    opentimers_vars.running = TRUE;
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
\brief get the current counter value of sctimer.

\returns the current counter value.
 */
PORT_TIMER_WIDTH opentimers_getValue(void){
    return sctimer_readCounter();
}

/**
\brief get the currentCompareValue variable of opentimer2.

\returns currentCompareValue.
 */
PORT_TIMER_WIDTH opentimers_getCurrentCompareValue(void){
    return opentimers_vars.currentCompareValue;
}

/**
\brief is the given timer running?

\returns isRunning variable.
 */
bool opentimers_isRunning(opentimers_id_t id){
    return opentimers_vars.timersBuf[id].isrunning;
}
// ========================== task ============================================

// ========================== callback ========================================

/**
\brief this is the callback function of opentimer.

This function is called when sctimer interrupt happens. The function looks the
whole timer buffer and find out the correct timer responding to the interrupt
and call the callback recorded for that timer.
 */
void opentimers_timer_callback(void){
    uint8_t i;
    uint8_t idToSchedule;
    PORT_TIMER_WIDTH timerGap;
    PORT_TIMER_WIDTH tempTimerGap;

    opentimers_vars.insideISR = TRUE;
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if (opentimers_vars.timersBuf[i].isrunning==TRUE){
            if (opentimers_vars.currentCompareValue == opentimers_vars.timersBuf[i].currentCompareValue){
                // this timer expired, mark as expired
                opentimers_vars.timersBuf[i].lastCompareValue    = opentimers_vars.timersBuf[i].currentCompareValue;
                if (opentimers_vars.timersBuf[i].wraps_remaining==0){
                    opentimers_vars.timersBuf[i].isrunning  = FALSE;
                    opentimers_vars.timersBuf[i].hasExpired = FALSE;
                    if (i==0){
                        opentimers_vars.timersBuf[0].callback(0);
                    } else {
                        scheduler_push_task((task_cbt)(opentimers_vars.timersBuf[i].callback),TASKPRIO_OPENTIMERS);
                        if (opentimers_vars.timersBuf[i].timerType==TIMER_PERIODIC){
                            opentimers_scheduleIn(
                                i,
                                opentimers_vars.timersBuf[i].duration,
                                TIME_TICS,
                                TIMER_PERIODIC,
                                opentimers_vars.timersBuf[i].callback
                            );
                        }
                    }
                } else {
                    opentimers_vars.timersBuf[i].wraps_remaining--;
                    if (opentimers_vars.timersBuf[i].wraps_remaining == 0){
                        opentimers_vars.timersBuf[i].currentCompareValue = (opentimers_vars.timersBuf[i].duration+opentimers_vars.timersBuf[i].lastCompareValue) & MAX_TICKS_IN_SINGLE_CLOCK;
                    } else {
                        opentimers_vars.timersBuf[i].currentCompareValue = opentimers_vars.timersBuf[i].lastCompareValue + MAX_TICKS_IN_SINGLE_CLOCK;
                    }
                }
            }
        }
    }
    opentimers_vars.lastCompareValue = opentimers_vars.currentCompareValue;

    // find the next timer to be fired
    i = 0;
    while (opentimers_vars.timersBuf[i].isrunning==FALSE && i<MAX_NUM_TIMERS){
        i++;
    }
    if(i<MAX_NUM_TIMERS){
        timerGap     = opentimers_vars.timersBuf[i].currentCompareValue+opentimers_vars.timersBuf[i].wraps_remaining*MAX_TICKS_IN_SINGLE_CLOCK-opentimers_vars.lastCompareValue;
        idToSchedule = i;
        for (i=idToSchedule+1;i<MAX_NUM_TIMERS;i++){
            if (opentimers_vars.timersBuf[i].isrunning){
                tempTimerGap = opentimers_vars.timersBuf[i].currentCompareValue-opentimers_vars.lastCompareValue;
                if (tempTimerGap < timerGap){
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            }
        }

        // reschedule the timer
        opentimers_vars.currentCompareValue = opentimers_vars.timersBuf[idToSchedule].currentCompareValue;
        opentimers_vars.lastCompare[opentimers_vars.index] = opentimers_vars.currentCompareValue;
        opentimers_vars.index = (opentimers_vars.index+1)&0x0F;
        sctimer_setCompare(opentimers_vars.currentCompareValue);
        opentimers_vars.running        = TRUE;
    } else {
        opentimers_vars.running        = FALSE;
    }
    opentimers_vars.insideISR      = FALSE;
}