#ifndef __TIMERS_H
#define __TIMERS_H

#include "stdint.h"

//=========================== define ==========================================

typedef enum {
   TIMER_PERIODIC,
   TIMER_ONESHOT,
} timer_type_t;

//=========================== typedef =========================================

typedef void (*timer_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

void timers_init();
void timers_start(uint8_t      id,
                  uint16_t     duration,
                  timer_type_t type,
                  timer_cbt    callback);
void timers_stop (uint8_t      id);

#endif
