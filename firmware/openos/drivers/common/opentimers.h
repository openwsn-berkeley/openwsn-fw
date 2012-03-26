/**
\brief Declaration of the "opentimers" driver.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/

#ifndef __OPENTIMERS_H
#define __OPENTIMERS_H

#include "openwsn.h"
#include "bsp_timer.h"

//=========================== define ==========================================

/// The number of timer that can run concurrently
#define MAX_NUM_TIMERS            10

#define TOO_MANY_TIMERS_ERROR     255

typedef void (*opentimers_cbt)(void);

//=========================== typedef =========================================

typedef enum {
   TIMER_PERIODIC,
   TIMER_ONESHOT,
} timer_type_t;

#define opentimer_id_t uint8_t

typedef struct {
   PORT_TIMER_WIDTH     period_ticks;       // period, in ticks
   PORT_TIMER_WIDTH     ticks_remaining;    // ticks remaining before elapses
   timer_type_t         type;               // periodic or one-shot
   bool                 isrunning;          // is running?
   opentimers_cbt       callback;           // function to call when elapses
   bool                 hasExpired;         // whether the callback has to be called
} opentimers_t;

//=========================== prototypes ======================================

void           opentimers_init();
opentimer_id_t opentimers_start(uint16_t       duration,
                                timer_type_t   type,
                                opentimers_cbt callback);
void           opentimers_setPeriod(opentimer_id_t id,
                                    uint16_t       newPeriod);
void           opentimers_stop(opentimer_id_t id);

#endif
