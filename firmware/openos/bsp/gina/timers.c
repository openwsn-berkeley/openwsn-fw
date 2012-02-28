#include "msp430x26x.h"
#include "string.h"
#include "timers.h"
#include "board.h"

//=========================== defines =========================================

#define TIMER_COUNT     7              // number of available timer;

//=========================== variables =======================================

typedef struct {
   uint16_t        period[TIMER_COUNT];
   timer_type_t    type[TIMER_COUNT];
   timer_cbt       callback[TIMER_COUNT];
} timers_vars_t;

timers_vars_t timers_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void timers_init() {
   
   // clear local variables
   memset(&timers_vars,0,sizeof(timers_vars));
   
   // source ACLK from 32kHz crystal
   BCSCTL3        |=  LFXT1S_0;

   //set CCRBx registers
   TBCCTL0         =  0;
   TBCCR0          =  0;
   TBCCTL1         =  0;
   TBCCR1          =  0;
   TBCCTL2         =  0;
   TBCCR2          =  0;
   TBCCTL3         =  0;
   TBCCR3          =  0;
   TBCCTL4         =  0;
   TBCCR4          =  0;
   TBCCTL5         =  0;
   TBCCR5          =  0;
   TBCCTL6         =  0;
   TBCCR6          =  0;

   //start TimerB
   TBCTL           =  MC_2+TBSSEL_1;             // continuous mode, from ACLK
}

void timers_start(uint8_t      id,
                  uint16_t     duration,
                  timer_type_t type,
                  timer_cbt    callback) {
   
   // register timer
   timers_vars.period[id]    = duration;
   timers_vars.type[id]      = type;
   timers_vars.callback[id]  = callback;
   
   // play with HW registers
   switch(id) {
      case 0:
         TBCCR0    =  TBR+timers_vars.period[id];
         TBCCTL0   =  CCIE;
         break;
      case 1:
         TBCCR1    =  TBR+timers_vars.period[id];
         TBCCTL1   =  CCIE;
         break;
      case 2:
         TBCCR2    =  TBR+timers_vars.period[id];
         TBCCTL2   =  CCIE;
         break;
      case 3:
         TBCCR3    =  TBR+timers_vars.period[id];
         TBCCTL3   =  CCIE;
         break;
      case 4:
         TBCCR4    =  TBR+timers_vars.period[id];
         TBCCTL4   =  CCIE;
         break;
      case 5:
         TBCCR5    =  TBR+timers_vars.period[id];
         TBCCTL5   =  CCIE;
         break;
      case 6:
         TBCCR6    =  TBR+timers_vars.period[id];
         TBCCTL6   =  CCIE;
         break;
   }
}

void timers_stop(uint8_t id) {
   
   // unregister timer
   timers_vars.period[id]    = 0;
   timers_vars.callback[id]  = NULL;
   
   // play with HW registers
   switch(id) {
      case 0:
         TBCCR0    =  0;
         TBCCTL0  &= ~CCIE;
         break;
      case 1:
         TBCCR1    =  0;
         TBCCTL1  &= ~CCIE;
         break;
      case 2:
         TBCCR2    =  0;
         TBCCTL2  &= ~CCIE;
         break;
      case 3:
         TBCCR3    =  0;
         TBCCTL3  &= ~CCIE;
         break;
      case 4:
         TBCCR4    =  0;
         TBCCTL4  &= ~CCIE;
         break;
      case 5:
         TBCCR5    =  0;
         TBCCTL5  &= ~CCIE;
         break;
      case 6:
         TBCCR6    =  0;
         TBCCTL6  &= ~CCIE;
         break;
   }
}

//=========================== private =========================================

//=========================== interrup handlers ===============================

// TimerB CCR0 interrupt service routine
#pragma vector = TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void) {
   if (timers_vars.type[0]==TIMER_PERIODIC) {
      TBCCR0           += timers_vars.period[0]; // continuous timer: schedule next instant
   } else {
      TBCCTL0           = 0;                     // stop the timer
      TBCCR0            = 0;
   }
   // call the callback
   timers_vars.callback[0]();
   // make sure CPU restarts after leaving interrupt
   __bic_SR_register_on_exit(CPUOFF);
}

// TimerB CCR1-6 interrupt service routine
#pragma vector = TIMERB1_VECTOR
__interrupt void TIMERB1_ISR (void) {
   uint16_t tbiv_temp   = TBIV;                  // read only once because accessing TBIV resets it
   switch (tbiv_temp) {
      case 0x0002: // timerB CCR1
         if (timers_vars.type[1]==TIMER_PERIODIC) {
            TBCCR1     += timers_vars.period[1]; // continuous timer: schedule next instant
         } else {
            TBCCTL1     = 0;                     // stop the timer
            TBCCR1      = 0;
         }
         // call the callback
         timers_vars.callback[1]();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
         break;
      case 0x0004: // timerB CCR2
         if (timers_vars.type[2]==TIMER_PERIODIC) {
            TBCCR2     += timers_vars.period[2]; // continuous timer: schedule next instant
         } else {
            TBCCTL2     = 0;                     // stop the timer
            TBCCR2      = 0;
         }
         // call the callback
         timers_vars.callback[2]();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
         break;
      case 0x0006: // timerB CCR3
         if (timers_vars.type[3]==TIMER_PERIODIC) {
            TBCCR3     += timers_vars.period[3]; // continuous timer: schedule next instant
         } else {
            TBCCTL3     = 0;                     // stop the timer
            TBCCR3      = 0;
         }
         // call the callback
         timers_vars.callback[3]();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
         break;
      case 0x0008: // timerB CCR4
         if (timers_vars.type[4]==TIMER_PERIODIC) {
            TBCCR4     += timers_vars.period[4]; // continuous timer: schedule next instant
         } else {
            TBCCTL4     = 0;                     // stop the timer
            TBCCR4      = 0;
         }
         // call the callback
         timers_vars.callback[4]();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
         break;
      case 0x000A: // timerB CCR5
         if (timers_vars.type[5]==TIMER_PERIODIC) {
            TBCCR5     += timers_vars.period[5]; // continuous timer: schedule next instant
         } else {
            TBCCTL5     = 0;                     // stop the timer
            TBCCR5      = 0;
         }
         // call the callback
         timers_vars.callback[5]();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
         break;
      case 0x000C: // timerB CCR6
         if (timers_vars.type[6]==TIMER_PERIODIC) {
            TBCCR6     += timers_vars.period[6]; // continuous timer: schedule next instant
         } else {
            TBCCTL6     = 0;                     // stop the timer
            TBCCR6      = 0;
         }
         // call the callback
         timers_vars.callback[6]();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
         break;
      default:
         while(1);                               // this should not happen
   }
}