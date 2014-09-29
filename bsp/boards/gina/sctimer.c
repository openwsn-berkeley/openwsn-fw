/**
\brief A timer module with only a single compare value. Can be used to replace
       the "bsp_timer" and "radiotimer" modules with the help of abstimer.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2012.
*/

#include "sctimer.h"
#include "msp430x26x.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uint8_t  running;
   uint16_t taiv;
} sctimers_vars_t;

sctimers_vars_t sctimers_vars;


//=========================== prototypes ======================================

void sctimer_setup();
void sctimer_start();

//=========================== public ==========================================

void sctimer_init() {
   sctimer_setup();
}

void sctimer_stop() {
   sctimer_setup();
}

void sctimer_schedule(uint16_t val) {
   
   if (sctimers_vars.running==0) {
      sctimers_vars.running=1;
      sctimer_start();
   }
   
   // load when to fire
   TACCR1   =  val;
   
   // enable interrupt
   TACCTL1  =  CCIE;
}

uint16_t sctimer_getValue() { 
   return TAR;
}

void sctimer_setCb(sctimer_cbt cb){
  // does nothing as it is done by IAR -- look at board.c
}

void sctimer_clearISR() {
   sctimers_vars.taiv = TAIV;//read taiv to clear the flags.
}
//=========================== private =========================================

void sctimer_setup() {
   // clear local variables
   memset(&sctimers_vars,0,sizeof(sctimers_vars_t));
   
   // ACLK sources from external 32kHz
   BCSCTL3 |= LFXT1S_0;
   
   // disable all compares
   TACCTL0  =  0;
   TACCR0   =  0;
   
   // CCR1 in compare mode (disabled for now)
   TACCTL1  =  0;
   TACCR1   =  0;
   
   // CCR2 in capture mode
   TACCTL2  =  0;
   TACCR2   =  0;
   
   // reset couter
   TAR      =  0;
}

void sctimer_start() {
   // start counting
   TACTL    =  MC_2+TASSEL_1;                    // continuous mode, clocked from ACLK
}