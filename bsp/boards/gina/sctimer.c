/**
\brief A timer module with only a single compare value. 

\author: Tengfei Chang <tengfei.chang@inria.fr> April 2017
*/

#include "sctimer.h"
#include "msp430x26x.h"
#include "sctimer.h"
#include "board.h"
#include "leds.h"

// ========================== define ==========================================

#define TIMERLOOP_THRESHOLD          0x4000     // 0.5 seconds @ 32768Hz clock
#define MINIMUM_COMPAREVALE_ADVANCE  3

// ========================== variable ========================================

typedef struct {
    sctimer_cbt         cb;
    sctimer_capture_cbt startFrameCb;
    sctimer_capture_cbt endFrameCb;
    uint8_t             f_SFDreceived;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars;

// ========================== private ==========================================

// ========================== protocol =========================================

/**
\brief Initialization sctimer.
*/
void sctimer_init(void){
    memset(&sctimer_vars, 0, sizeof(sctimer_vars_t));
    
    // source ACLK from 32kHz crystal
    BCSCTL3 |= LFXT1S_0;
   
    // CCR2 in compare mode (disabled for now)
    TBCCTL2  =  0;
    TBCCR2   =  0;
   
    // start counting
    TBCTL    =  MC_2+TBSSEL_1;                    // continue mode, clocked from ACLK
}

void sctimer_set_callback(sctimer_cbt cb){
    sctimer_vars.cb = cb;
}

void sctimer_setStartFrameCb(sctimer_capture_cbt cb){
    sctimer_vars.startFrameCb = cb;
}

void sctimer_setEndFrameCb(sctimer_capture_cbt cb){
    sctimer_vars.endFrameCb = cb;
}

/**
\brief set compare interrupt
*/
void sctimer_setCompare(PORT_TIMER_WIDTH val){
    TBCCTL2             =  CCIE;
    if (TBR - val < TIMERLOOP_THRESHOLD){
        // the timer is already late, schedule the ISR right now manually 
        // setting the interrupt flag triggers an interrupt
        TBCCTL2          |=  CCIFG;
    } else {
        if (val-TBR<MINIMUM_COMPAREVALE_ADVANCE){
            // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
            // setting the interrupt flag triggers an interrupt
            TBCCTL2          |=  CCIFG;
        } else {
            // schedule the timer at val
            TBCCR2            =  val;
        }
    }
}

/**
\brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH sctimer_readCounter(void){
    return TBR;
}

void sctimer_enable(void){
    TBCCTL2 |=  CCIE;
}

void sctimer_disable(void){
    TBCCTL2 &= ~CCIE;
}

// ========================== private =========================================


//=========================== interrupt handlers ==============================

/**
\brief TimerB CCR1-6 interrupt service routine
*/
kick_scheduler_t sctimer_isr() {
   PORT_TIMER_WIDTH tbiv_local;
   
   // reading TBIV returns the value of the highest pending interrupt flag
   // and automatically resets that flag. We therefore copy its value to the
   // tbiv_local local variable exactly once. If there is more than one 
   // interrupt pending, we will reenter this function after having just left
   // it.
   
   tbiv_local = TBIV;
   
   switch (tbiv_local) {
      case 0x0002: // CCR1 fires
         break;
      case 0x0004: // CCR2 fires
         if (sctimer_vars.cb!=NULL) {
            sctimer_vars.cb();
            // kick the OS
            return KICK_SCHEDULER;
         }
         break;
      case 0x0006: // CCR3 fires
         break;
      case 0x0008: // CCR4 fires
         break;
      case 0x000a: // CCR5 fires
         break;
      case 0x000c: // CCR6 fires
         break;
      case 0x000e: // timer overflow
         break;
   }
   return DO_NOT_KICK_SCHEDULER;
}
