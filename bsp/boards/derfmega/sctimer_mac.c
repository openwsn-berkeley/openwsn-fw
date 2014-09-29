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
   PRR0 &= ~(1<<PRTIM2); // turn on timer 2 for crystal
   SCCR0 = 0b00111000; // enable symbol counter, 32KHz clock, auto timestamp, no relative compare
   SCCR1 = 0; // no backoff slot counter
   SCIRQM = 1; // only interrupt on compare match 1
   ASSR |= (1<<AS2); // enable 32KHz crystal
}

void sctimer_schedule(PORT_TIMER_WIDTH val) {
   // load when to fire
   SCOCR1HH = (uint8_t)(val>>24);
   SCOCR1HL = (uint8_t)(val>>16);
   SCOCR1LH = (uint8_t)(val>>8);
   SCOCR1LL = (uint8_t)(val);
  
   // enable interrupt
   SCIRQM |= 1; // compare match 1 enable interrupts
   
   // wait for it to synchronize
   while(SCBSY & 1);
}

PORT_TIMER_WIDTH sctimer_getValue() {
	PORT_TIMER_WIDTH val = SCCNTLL;
	val |= ((PORT_TIMER_WIDTH)SCCNTLH<<8);
	val |= ((PORT_TIMER_WIDTH)SCCNTHL<<16);
	val |= ((PORT_TIMER_WIDTH)SCCNTHH<<24);
   return val;
}

void sctimer_clearISR() {
	SCIRQM &= 0xFE;
}


void sctimer_setCb(sctimer_cbt cb){
}
//=========================== private =========================================