/**
\brief Declaration of the "opentimers" driver.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/

#ifndef __OPENTIMERS_H
#define __OPENTIMERS_H

#include "openwsn.h"
#include "bsp_timer.h"

//=========================== define ==========================================

#define MAX_NUM_TIMERS            10
#define TOO_MANY_TIMERS_ERROR     255

typedef void (*opentimers_cbt)(void);

//=========================== typedef =========================================

typedef enum {
   TIMER_PERIODIC  = 0,
   TIMER_ONESHOT   = 1,
} timer_type_t;

#define opentimer_id_t uint8_t

typedef struct {
   opentimer_id_t  id;                 // unique ID of the timer
   uint32_t        period;             // timeout, in ms
   uint32_t        timer_remaining;    // time remaining before elapses
   timer_type_t    type;               // periodic or one-shot
   bool            isrunning;          // is running?
   opentimers_cbt  callback;           // function to call when elapses
   bool            hasFired;           // whether the callback has to be called
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
