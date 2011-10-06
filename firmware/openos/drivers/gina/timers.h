#ifndef __TIMERS_H
#define __TIMERS_H

/**
\addtogroup drivers
\{
\addtogroup Timers
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

//timer ids
enum {
   TIMER_RES                 = 0,                // mapped onto timerB CCR0
   TIMER_RPL                 = 1,                // mapped onto timerB CCR1
   TIMER_TCP                 = 2,                // mapped onto timerB CCR2
   TIMER_UDPTIMER            = 3,                // mapped onto timerB CCR3
   TIMER_B4                  = 4,                // mapped onto timerB CCR4
   TIMER_B5                  = 5,                // mapped onto timerB CCR5
   TIMER_B6                  = 6,                // mapped onto timerB CCR6
   TIMER_COUNT               = 7,                // number of available timers
};

//=========================== typedef =========================================

typedef struct {
   uint16_t period[TIMER_COUNT];
   bool     continuous[TIMER_COUNT];
} timers_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void timer_init();
void timer_start(uint8_t timer_id, uint16_t duration, bool continuous);
void timer_startOneShot(uint8_t timer_id, uint16_t duration);
void timer_startPeriodic(uint8_t timer_id, uint16_t duration);
void timer_stop(uint8_t timer_id);

// functions to call when timer fires
void timer_res_fired();
void timer_rpl_fired();
void timer_tcp_fired();
void timer_appudptimer_fired();

/**
\}
\}
*/

#endif
