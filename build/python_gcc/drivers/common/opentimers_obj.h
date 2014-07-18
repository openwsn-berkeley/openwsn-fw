/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:01.192213.
*/
/**
\brief Declaration of the "opentimers" driver.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/

#ifndef __OPENTIMERS_H
#define __OPENTIMERS_H

#include "openwsn_obj.h"

/**
\addtogroup drivers
\{
\addtogroup OpenTimers
\{
*/

//=========================== define ==========================================

/// Maximum number of timers that can run concurrently
#define MAX_NUM_TIMERS            10

#define MAX_TICKS_IN_SINGLE_CLOCK ((PORT_TIMER_WIDTH)0xFFFFFFFF)

#define TOO_MANY_TIMERS_ERROR     255

#define opentimer_id_t uint8_t

typedef void (*opentimers_cbt)(OpenMote* self);

//=========================== typedef =========================================

typedef enum {
   TIMER_PERIODIC,
   TIMER_ONESHOT,
} timer_type_t;

/*the time can be in tics or in ms*/
typedef enum {
   TIME_MS,
   TIME_TICS,
} time_type_t;

typedef struct {
   uint32_t             period_ticks;       // total number of clock ticks
   PORT_TIMER_WIDTH     ticks_remaining;    // ticks remaining before elapses
   uint16_t             wraps_remaining;    // the clock register is 16 bit, and can't count beyond 32k...
                                            // so period_ticks = wraps_remaining*(32k or uint16_t)
   timer_type_t         type;               // periodic or one-shot
   bool                 isrunning;          // is running?
   opentimers_cbt       callback;           // function to call when elapses
   bool                 hasExpired;         // whether the callback has to be called
} opentimers_t;

//=========================== module variables ================================

typedef struct {
   opentimers_t         timersBuf[MAX_NUM_TIMERS];
   bool                 running;
   PORT_TIMER_WIDTH     currentTimeout; // current timeout, in ticks
} opentimers_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void opentimers_init(OpenMote* self);
opentimer_id_t opentimers_start(OpenMote* self, uint32_t       duration,
                                timer_type_t   type,
                                time_type_t timetype,
                                opentimers_cbt callback);
void opentimers_setPeriod(OpenMote* self, opentimer_id_t id,time_type_t timetype, uint32_t       newPeriod);
void opentimers_stop(OpenMote* self, opentimer_id_t id);
void opentimers_restart(OpenMote* self, opentimer_id_t id);

void opentimers_sleepTimeCompesation(OpenMote* self, uint16_t sleepTime);

/**
\}
\}
*/

#endif
