/**
\brief Declaration of the "opentimers" driver.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
*/

#ifndef __OPENTIMERS_H
#define __OPENTIMERS_H

#include "opendefs.h"

/**
\addtogroup drivers
\{
\addtogroup OpenTimers
\{
*/

//=========================== define ==========================================

/// Maximum number of timers that can run concurrently
#define MAX_NUM_TIMERS             10
#define MAX_TICKS_IN_SINGLE_CLOCK  (uint32_t)(((PORT_TIMER_WIDTH)0xFFFFFFFF)>>1)
#define ERROR_NO_AVAILABLE_ENTRIES 255
#define MAX_DURATION_ISR           33 // 33@32768Hz = 1ms
#define opentimers_id_t            uint8_t

#define TIMER_INHIBIT              0
#define TIMER_TSCH                 1
#define TIMER_GENERAL_PURPOSE      255

#define TIMER_NUMBER_NON_GENERAL   2

#define SPLITE_TIMER_DURATION     15 // in ticks

typedef void (*opentimers_cbt)(opentimers_id_t id);

//=========================== typedef =========================================

typedef enum {
   TIMER_PERIODIC,
   TIMER_ONESHOT,
} timer_type_t;

typedef enum {
   TIME_MS,
   TIME_TICS,
} time_type_t;

typedef struct {
   uint32_t             duration;           // the duration that set by timer, in ticks
   PORT_TIMER_WIDTH     currentCompareValue;// the current compare value
   uint16_t             wraps_remaining;    // the number of wraps timer is going to be fired after
   PORT_TIMER_WIDTH     lastCompareValue;   // the previous compare value
   bool                 isrunning;          // is running?
   bool                 isUsed;             // true when this entry is occupied
   timer_type_t         timerType;          // the timer type
   bool                 hasExpired;         // in case there are more than one interrupt occur at same time
   opentimers_cbt       callback;           // function to call when elapses
} opentimers_t;

//=========================== module variables ================================

typedef struct {
   opentimers_t         timersBuf[MAX_NUM_TIMERS];
   bool                 running;
   PORT_TIMER_WIDTH     currentCompareValue;// current timeout, in ticks
   PORT_TIMER_WIDTH     lastCompareValue;   // last timeout, in ticks. This is the reference time to calculate the next to be expired timer.
   bool                 insideISR;          // whether the function of opentimer is called inside of ISR or not
   bool                 timerSplited;
} opentimers_vars_t;

//=========================== prototypes ======================================

void             opentimers_init(void);
opentimers_id_t  opentimers_create(uint8_t priority);
void             opentimers_scheduleIn(opentimers_id_t      id,
                                       uint32_t            duration,
                                       time_type_t         uint_type,
                                       timer_type_t        timer_type,
                                       opentimers_cbt      cb);
void             opentimers_scheduleAbsolute(opentimers_id_t      id,
                                              uint32_t            duration,
                                              PORT_TIMER_WIDTH    reference ,
                                              time_type_t         uint_type,
                                              opentimers_cbt      cb);
void             opentimers_cancel(opentimers_id_t id);
bool             opentimers_destroy(opentimers_id_t id);

PORT_TIMER_WIDTH opentimers_getValue(void);
PORT_TIMER_WIDTH opentimers_getCurrentCompareValue(void);
bool             opentimers_isRunning(opentimers_id_t id);
/**
\}
\}
*/

#endif