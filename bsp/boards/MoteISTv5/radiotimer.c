/**
\brief MoteISTv5-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
\author Diogo Guerra <diogoguerra@ist.utl.pt>, <dy090.guerra@gmail.com>, July 2015.
*/

#include "hal_MoteISTv5.h"
#include "radiotimer.h"
#include "leds.h"

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflowCb;
   radiotimer_compare_cbt    compareCb;
   radiotimer_capture_cbt    startFrameCb;
   radiotimer_capture_cbt    endFrameCb;
   uint8_t                   f_SFDreceived;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================


//=========================== definitions =====================================


#define SFD       0x02  // P1.1
#define RST       0x40  // P9.6
#define VREG      0x80  // P9.7

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.overflowCb     = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.compareCb      = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_vars.startFrameCb   = cb;
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_vars.endFrameCb     = cb;
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
   // radio's SFD pin connected to P1.1
   P1DIR       &= ~SFD;          // [P1.1] radio SFD:    input
   P1SEL       &= ~SFD;          // [P1.1] radio SFD:    in INTERRUPT MODE
   P1IES       &= ~SFD;          // [P1.1] radio SFD:    low->high
   P1IFG       &= ~SFD;          // [P1.1] radio SFD:    clear interrupt flag
   P1IE        |=  SFD;          // [P1.1] radio SFD:    interrupt enabled

   // CCR0 contains period of counter
   // do not interrupt when counter reaches TACCR0, but when it resets
   TA0CCR0   =  period-1;
   
   // CCR1 in capture mode
   // TA0CCTL1  =  CM_3+SCS+CAP+CCIE;
   // TA0CCR1   =  0;
   // We can't use this to CAPTURE the Radio SFD since it is connected to pin
   // P1.1 which correspond to timerRegister CCR0 used to reset periodically the timer
   // -- Instead we opt to use PORT1 ISR instead of bypassing it to TA0CCR0
   
   // CCR2 in compare mode (disabled for now)
   TA0CCTL2  =  0;
   TA0CCR2   =  0;
   
   // start counting
   TA0CTL    =  TAIE+TACLR;                       // interrupt when counter resets
   TA0CTL   |=  MC_1+TASSEL_1;                    // up mode, clocked from ACLK
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
   return TA0R;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
   TA0CCR0   =  period;
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
   return TA0CCR0;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
   // offset when to fire
   TA0CCR2   =  offset;
   
   // enable compare interrupt (this also cancels any pending interrupts)
   TA0CCTL2  =  CCIE;
}

void radiotimer_cancel() {
   // reset compare value (also resets interrupt flag)
   TA0CCR2   =  0;
   
   // disable compare interrupt
   TA0CCTL2 &= ~CCIE;
}

//===== capture

inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
   return TA0R;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

/**
\brief TimerA0 CCR1-6 interrupt service routine
*/
kick_scheduler_t radiotimer_isr() {
   PORT_RADIOTIMER_WIDTH tbiv_local;
   
   // reading TA0IV returns the value of the highest pending interrupt flag
   // and automatically resets that flag. We therefore copy its value to the
   // tbiv_local local variable exactly once. If there is more than one 
   // interrupt pending, we will reenter this function after having just left
   // it.
   tbiv_local = TA0IV;
   
   switch (tbiv_local) {
      case 0x0002: // CCR1 fires
         //Replaced with Radio SFD Workaround
         // if (TA0CCTL1 & CCI) {
         //    // SFD pin is high: this was the start of a frame
         //    if (radiotimer_vars.startFrameCb!=NULL) {
         //       radiotimer_vars.startFrameCb(TA0CCR1);
         //       radiotimer_vars.f_SFDreceived = 1;
         //       // kick the OS
         //       return KICK_SCHEDULER;
         //    }
         // } else {
         //    // SFD pin is low: this was the end of a frame
         //    if (radiotimer_vars.endFrameCb!=NULL) {
         //       if (radiotimer_vars.f_SFDreceived == 1) {
         //          radiotimer_vars.endFrameCb(TA0CCR1);
         //          radiotimer_vars.f_SFDreceived = 0;
         //       }
         //       TA0CCTL1 &= ~COV;
         //       TA0CCTL1 &= ~CCIFG;
         //       // kick the OS
         //       return KICK_SCHEDULER;
         //    }
         // }
         //This should never happen since TA1 is our reset timer
         leds_error_blink();
         break;
      case 0x0004: // CCR2 fires
         if (radiotimer_vars.compareCb!=NULL) {
            radiotimer_vars.compareCb();
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
         if (radiotimer_vars.overflowCb!=NULL) {
            radiotimer_vars.overflowCb();
            // kick the OS
            return KICK_SCHEDULER;
         }
         break;
   }
   return DO_NOT_KICK_SCHEDULER;
}


/**
\brief Radio SFD Workaround
*/
kick_scheduler_t radiotimer_isr_sfd(void) {
   PORT_RADIOTIMER_WIDTH now;
   now = radiotimer_getCapturedTime();  //get timer (not ideal since delay occurs from real pin up time)
   // Change interrupt transition now, because SFD can go down while we are in int. routine if packet is small
   P1IES     ^=  SFD;   // arm in opposite transition
   P1IFG    &=   ~SFD;  // clear flag to ensure that new IFG was not generated by previous instruction

   if (!(P1IES & SFD)) {  //Logic is changed cause of previous instruction
      // high->low just happened
      if (radiotimer_vars.endFrameCb!=NULL) {
         radiotimer_vars.endFrameCb(now);
         radiotimer_vars.f_SFDreceived = 0;
      }
   } else {
      // low->high just happened
      if (radiotimer_vars.startFrameCb!=NULL) {
         radiotimer_vars.startFrameCb(now);
         radiotimer_vars.f_SFDreceived = 1;
      }
   }
   return KICK_SCHEDULER;
}