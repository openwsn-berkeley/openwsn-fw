/**
\brief A timer module with only a single compare value. Can be used to replace
       the "bsp_timer" and "radiotimer" modules with the help of abstimer.

on the derfmega we use timer 2 with asynchronous operation

\author Kevin Weekly <kweekly@eecs.berkeley.edu>, June 2012.
*/

#include <avr/io.h>
#include "board.h"
#include "sctimer.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void sctimer_init() {
   // enable power
   PRR0 &= ~(1<<PRTIM2);
   
   TIMSK2 = 0; // disable interrupt
   
   TCCR2A = 0x02; // Clear timer on match with OCRA
   TCCR2B = 0x01; // 1:1 prescaler   
   ASSR = 0b00100000;       // go asnychronous
   
   TIMSK2 |= 0x02; // enable interrupt
}

void sctimer_schedule(PORT_TIMER_WIDTH val) {
   // load when to fire
   OCR2A   =  val;
  
   // enable interrupt
   TIMSK2 |= 0x02; // compare match A enable interrupts
   
   // wait for it to synchronize
   while(ASSR & (1<<OCR2AUB));
}

PORT_TIMER_WIDTH sctimer_getValue() {
   return TCNT2;
}

void sctimer_clearISR() {
	TIFR2 |= (1<<OCF2A);
}


void sctimer_setCb(sctimer_cbt cb){
   
}
//=========================== private =========================================