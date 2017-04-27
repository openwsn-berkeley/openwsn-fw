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
#define MAX_NUM_TIMERS            10
#define MAX_TICKS_NUMBER          ((PORT_TIMER_WIDTH)0xFFFFFFFF)
#define TOO_MANY_TIMERS_ERROR     255
#define MAX_DURATION_ISR          33 // 33@32768Hz = 1ms
#define opentimers_id_t uint8_t

typedef void (*opentimers_cbt)(void);

//=========================== typedef =========================================

typedef enum {
   TIME_MS,
   TIME_TICS,
} time_type_t;

typedef struct {
   uint32_t             currentCompareValue;// total number of clock ticks
   uint32_t             lastCompareValue;   // the previous compare value
   bool                 isrunning;          // is running?
   bool                 isUsed;             // true when this entry is occupied
   bool                 hasExpired;         // in case there are more than one interrupt occur at same time
   uint8_t              priority;           // high priority timer could take over the compare timer scheduled early than it for TIMERTHRESHOLD ticks.
   opentimers_cbt      callback;           // function to call when elapses
} opentimers_t;

//=========================== module variables ================================

typedef struct {
   opentimers_t        timersBuf[MAX_NUM_TIMERS];
   bool                 running;
   uint32_t             currentTimeout; // current timeout, in ticks
   uint32_t             lastTimeout;    // last timeout, in ticks. This is the reference time to calculate the next to be expired timer.
} opentimers_vars_t;

//=========================== prototypes ======================================

void             opentimers_init(void);
opentimers_id_t opentimers_create(void);
void             opentimers_scheduleRelative(opentimers_id_t    id, 
                                              uint32_t            duration,
                                              time_type_t         uint_type, 
                                              opentimers_cbt     cb);
void             opentimers_scheduleAbsolute(opentimers_id_t    id, 
                                              uint32_t            duration, 
                                              uint32_t            reference , 
                                              time_type_t         uint_type, 
                                              opentimers_cbt     cb);
void             opentimers_cancel(opentimers_id_t id);
bool             opentimers_destroy(opentimers_id_t id);

uint32_t         opentimers_getValue(opentimers_id_t id);
uint32_t         opentimers_getCurrentTimeout(void);
bool             opentimers_isRunning(opentimers_id_t id);
void             opentimers_setPriority(opentimers_id_t id, uint8_t priority);
/**
\}
\}
*/

#endif














