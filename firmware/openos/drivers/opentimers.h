#ifndef __OPENTIMERS_H
#define __OPENTIMERS_H

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
   TIMER_COAP                = 3,                // mapped onto timerB CCR3
   TIMER_B4                  = 4,                // mapped onto timerB CCR4
   TIMER_B5                  = 5,                // mapped onto timerB CCR5
   TIMER_B6                  = 6,                // mapped onto timerB CCR6
   TIMER_COUNT               = 7,                // number of available timers
};

//=========================== typedef =========================================

typedef struct {
   uint16_t period[TIMER_COUNT];
   bool     continuous[TIMER_COUNT];
} opentimers_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void opentimers_init();
void opentimers_start(uint8_t timer_id, uint16_t duration, bool continuous);
void opentimers_startOneShot(uint8_t timer_id, uint16_t duration);
void opentimers_startPeriodic(uint8_t timer_id, uint16_t duration);
void opentimers_stop(uint8_t timer_id);

// functions to call when timer fires
void opentimers_res_fired();
void opentimers_rpl_fired();
void opentimers_tcp_fired();
void opentimers_coap_fired();

/**
\}
\}
*/

#endif
