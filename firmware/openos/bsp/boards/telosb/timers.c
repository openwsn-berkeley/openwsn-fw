/**
\brief TelosB-specific definition of the "timers" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "string.h"
#include "timers.h"
#include "board.h"

//=========================== defines =========================================

#define TIMER_COUNT     3              // number of available timer;

//=========================== variables =======================================

typedef struct {
   uint16_t        period[TIMER_COUNT];
   timer_type_t    type[TIMER_COUNT];
   timer_cbt       timers_cb[TIMER_COUNT];
} timers_vars_t;

timers_vars_t timers_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void timers_init() {
   
   // clear local variables
   memset(&timers_vars,0,sizeof(timers_vars));
   
   //set CCRBx registers
   TACCTL0         =  0;
   TACCR0          =  0;
   TACCTL1         =  0;
   TACCR1          =  0;
   TACCTL2         =  0;
   TACCR2          =  0;

   //start TimerA
   TACTL           =  MC_2+TASSEL_1;             // continuous mode, from ACLK
}

void timers_start(uint8_t      id,
                  uint16_t     duration,
                  timer_type_t type,
                  timer_cbt    callback) {
   
   // register timer
   timers_vars.period[id]    = duration;
   timers_vars.type[id]      = type;
   timers_vars.timers_cb[id] = callback;
   
   // play with HW registers
   switch(id) {
      case 0:
         TACCR0    =  TAR+timers_vars.period[id];
         TACCTL0   =  CCIE;
         break;
      case 1:
         TACCR1    =  TAR+timers_vars.period[id];
         TACCTL1   =  CCIE;
         break;
      case 2:
         TACCR2    =  TAR+timers_vars.period[id];
         TACCTL2   =  CCIE;
         break;
   }
}

void timers_stop(uint8_t id) {
   
   // unregister timer
   timers_vars.period[id]    = 0;
   timers_vars.timers_cb[id] = NULL;
   
   // play with HW registers
   switch(id) {
      case 0:
         TACCR0    =  0;
         TACCTL0  &= ~CCIE;
         break;
      case 1:
         TACCR1    =  0;
         TACCTL1  &= ~CCIE;
         break;
      case 2:
         TACCR2    =  0;
         TACCTL2  &= ~CCIE;
         break;
   }
}

//=========================== private =========================================

//=========================== interrup handlers ===============================

uint8_t timer_isr_0() {
   if (timers_vars.type[0]==TIMER_PERIODIC) {
      TACCR0           += timers_vars.period[0]; // continuous timer: schedule next instant
   } else {
      TACCTL0           = 0;                     // stop the timer
      TACCR0            = 0;
   }
   // call the callback
   timers_vars.timers_cb[0]();
   // kick the OS
   return 1;
}

uint8_t timer_isr_1() {
   uint16_t taiv_temp   = TAIV;                  // read only once because accessing TAIV resets it
   switch (taiv_temp) {
      case 0x0002: // timerA CCR1
         if (timers_vars.type[1]==TIMER_PERIODIC) {
            TACCR1     += timers_vars.period[1]; // continuous timer: schedule next instant
         } else {
            TACCTL1     = 0;                     // stop the timer
            TACCR1      = 0;
         }
         // call the callback
         timers_vars.timers_cb[1]();
         // kick the OS
         return 1;
         break;
      case 0x0004: // timerA CCR2
         if (timers_vars.type[2]==TIMER_PERIODIC) {
            TACCR2     += timers_vars.period[2]; // continuous timer: schedule next instant
         } else {
            TACCTL2     = 0;                     // stop the timer
            TACCR2      = 0;
         }
         // call the callback
         timers_vars.timers_cb[2]();
         // kick the OS
         return 1;
         break;
      default:
         while(1);                               // this should not happen
   }
}