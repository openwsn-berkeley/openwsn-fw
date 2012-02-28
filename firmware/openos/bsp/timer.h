#ifndef __TIMERS_H
#define __TIMERS_H

#include "stdint.h"

//=========================== define ==========================================

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

#endif
