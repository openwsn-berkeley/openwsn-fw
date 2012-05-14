/**
\brief A timer module with only a single compare value. Can be used to replace
       the "bsp_timer" and "radiotimer" modules with the help of abstimer.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2012.
*/

#include "io430.h"
#include "sctimer.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void sctimer_init() {
   
   // ACLK sources from internal 12kHz VLO
   BCSCTL3 |= LFXT1S_2;
   
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
   
   // start counting
   TACTL    =  MC_2+TASSEL_1;                    // continuous mode, clocked from ACLK
}

void sctimer_schedule(uint16_t val) {
   // load when to fire
   TACCR1   =  val;
   
   // enable interrupt
   TACCTL1  =  CCIE;
}

uint16_t sctimer_getValue() {
   return TAR;
}


void sctimer_setCb(sctimer_cbt cb){
//does nothing as it is done by IAR -- look at board.c
}
//=========================== private =========================================