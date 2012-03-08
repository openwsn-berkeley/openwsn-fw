#ifndef __BSP_TIMERS_H
#define __BSP_TIMERS_H

#include "stdint.h"

//=========================== define ==========================================

//timer ids
enum {
   TIMER_RES                 = 0,                // mapped onto timerB CCR0
   TIMER_RPL                 = 1,                // mapped onto timerB CCR1
   TIMER_COAP                = 2,                // mapped onto timerB CCR2
   TIMER_TCP                 = 3,                // mapped onto timerB CCR3
   TIMER_B4                  = 4,                // mapped onto timerB CCR4
   TIMER_B5                  = 5,                // mapped onto timerB CCR5
   TIMER_B6                  = 6,                // mapped onto timerB CCR6
   TIMER_COUNT               = 7,                // number of available timers
};

typedef enum {
   TIMER_PERIODIC  = 0,
   TIMER_ONESHOT   = 1,
} timer_type_t;

//=========================== typedef =========================================

typedef void (*timer_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

void    timers_init();
void    timers_start(uint8_t      id,
                     uint16_t     duration,
                     timer_type_t type,
                     timer_cbt    callback);
void    timers_stop(uint8_t      id);

uint8_t timer_isr_0();
uint8_t timer_isr_1();

#endif
