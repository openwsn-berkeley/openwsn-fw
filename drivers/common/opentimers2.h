/**
\brief Declaration of the "opentimers2" driver.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
*/

#ifndef __OPENTIMERS2_H
#define __OPENTIMERS2_H

#include "opendefs.h"

/**
\addtogroup drivers
\{
\addtogroup OpenTimers2
\{
*/

//=========================== define ==========================================

/// Maximum number of timers that can run concurrently
#define MAX_NUM_TIMERS            10
#define MAX_TICKS_NUMBER          ((PORT_TIMER_WIDTH)0xFFFFFFFF)
#define TOO_MANY_TIMERS_ERROR     255
#define MAX_DURATION_ISR          33 // 33@32768Hz = 1ms
#define opentimers2_id_t uint8_t

typedef void (*opentimers2_cbt)(void);

//=========================== typedef =========================================

typedef struct {
   uint32_t             currentCompareValue;// total number of clock ticks
   uint32_t             lastCompareValue;   // the previous compare value
   bool                 isrunning;          // is running?
   bool                 isUsed;             // true when this entry is occupied
   bool                 hasExpired;         // in case there are more than one interrupt occur at same time
   uint8_t              priority;           // high priority timer could take over the compare timer scheduled early than it for TIMERTHRESHOLD ticks.
   opentimers2_cbt      callback;           // function to call when elapses
} opentimers2_t;

//=========================== module variables ================================

typedef struct {
   opentimers2_t        timersBuf[MAX_NUM_TIMERS];
   bool                 running;
   uint32_t             currentTimeout; // current timeout, in ticks
   uint32_t             lastTimeout;    // last timeout, in ticks. This is the reference time to calculate the next to be expired timer.
} opentimers2_vars_t;

//=========================== prototypes ======================================

void             opentimers2_init(void);
opentimers2_id_t opentimers2_create(uint8_t priority);
void             opentimers2_scheduleRelative(opentimers2_id_t    id, 
                                              uint32_t            duration,
                                              time_type_t         uint_type, 
                                              opentimers2_cbt     cb);
void             opentimers2_scheduleAbsolute(opentimers2_id_t    id, 
                                              uint32_t            duration, 
                                              uint32_t            reference , 
                                              time_type_t         uint_type, 
                                              opentimers2_cbt     cb);
void             opentimers2_cancel(opentimers2_id_t id);
bool             opentimers2_destroy(opentimers2_id_t id);

uint32_t         opentimers2_getValue(opentimers2_id_t id);
uint32_t         opentimers2_getCurrentTimeout();
bool             opentimers2_isRunning(opentimers2_id_t id);
/**
\}
\}
*/

#endif
