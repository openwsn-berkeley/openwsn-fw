#include "msp430x26x.h"
#include "opentimers.h"
#include "scheduler.h"

//=========================== variables =======================================

opentimers_vars_t opentimers_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void opentimers_init() {
   uint8_t i;
   for(i=0;i<TIMER_COUNT;i++) {
      opentimers_start(i, 0, FALSE);
   }
   
   BCSCTL3 |= LFXT1S_0;                          // source ACLK from 32kHz crystal

   //set CCRBx registers
   TBCCTL0  = 0;
   TBCCR0   = 0;
   TBCCTL1  = 0;
   TBCCR1   = 0;
   TBCCTL2  = 0;
   TBCCR2   = 0;
   TBCCTL3  = 0;
   TBCCR3   = 0;
   TBCCTL4  = 0;
   TBCCR4   = 0;
   TBCCTL5  = 0;
   TBCCR5   = 0;
   TBCCTL6  = 0;
   TBCCR6   = 0;

   //start TimerB on 32kHz ACLK
   TBCTL    = MC_2+TBSSEL_1;                     // continuous mode, using ACLK
}

void opentimers_startOneShot(uint8_t timer_id, uint16_t duration) {
   opentimers_start(timer_id, duration, FALSE);
}

void opentimers_startPeriodic(uint8_t timer_id, uint16_t duration) {
   opentimers_start(timer_id, duration, TRUE);
}

void opentimers_stop(uint8_t timer_id) {
   opentimers_vars.period[timer_id] = 0;
   opentimers_vars.continuous[timer_id] = 0;
   switch(timer_id) {
      case 0:
         TBCCR0   =  0;
         TBCCTL0 &= ~CCIE;
         break;
      case 1:
         TBCCR1   =  0;
         TBCCTL1 &= ~CCIE;
         break;
      case 2:
         TBCCR2   =  0;
         TBCCTL2 &= ~CCIE;
         break;
      case 3:
         TBCCR3   =  0;
         TBCCTL3 &= ~CCIE;
         break;
      case 4:
         TBCCR4   =  0;
         TBCCTL4 &= ~CCIE;
         break;
      case 5:
         TBCCR5   =  0;
         TBCCTL5 &= ~CCIE;
         break;
      case 6:
         TBCCR6   =  0;
         TBCCTL6 &= ~CCIE;
         break;
   }
}

//=========================== private =========================================

void opentimers_start(uint8_t timer_id, uint16_t duration, bool continuous) {
   opentimers_vars.period[timer_id]     = duration;
   opentimers_vars.continuous[timer_id] = continuous;
   switch(timer_id) {
      case 0:
         TBCCR0   = TBR+opentimers_vars.period[timer_id];
         TBCCTL0  = CCIE;
         break;
      case 1:
         TBCCR1   = TBR+opentimers_vars.period[timer_id];
         TBCCTL1  = CCIE;
         break;
      case 2:
         TBCCR2   = TBR+opentimers_vars.period[timer_id];
         TBCCTL2  = CCIE;
         break;
      case 3:
         TBCCR3   = TBR+opentimers_vars.period[timer_id];
         TBCCTL3  = CCIE;
         break;
      case 4:
         TBCCR4   = TBR+opentimers_vars.period[timer_id];
         TBCCTL4  = CCIE;
         break;
      case 5:
         TBCCR5   = TBR+opentimers_vars.period[timer_id];
         TBCCTL5  = CCIE;
         break;
      case 6:
         TBCCR6   = TBR+opentimers_vars.period[timer_id];
         TBCCTL6  = CCIE;
         break;
   }
}

