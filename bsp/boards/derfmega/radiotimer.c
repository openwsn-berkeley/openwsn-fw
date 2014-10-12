/**
\brief derfmega-specific definition of the "radiotimer" bsp module.

using the MAC timer with some trickery

\author Kevin Weekly June 2012.
*/

#include <avr/io.h>
#include "string.h"
#include "radiotimer.h"

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

uint8_t radiotimer_overflow_isr();
uint8_t radiotimer_compare_isr();
//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.overflow_cb    = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.compare_cb     = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   while(1);
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   while(1);
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
   
   PRR0 &= ~(1<<PRTIM2); // turn on timer 2 for crystal
   SCCR0 = (SCCR0 | 0b00110110) & 0b11111110; // enable symbol counter, 32KHz clock, absolute compare 1,
											  // relative compare 2, relative compare 3
   SCCR1 = 0; // no backoff slot counter
   ASSR |= (1<<AS2); // enable 32KHz crystal

   //set compare registers
   *((PORT_RADIOTIMER_WIDTH *)(&SCOCR2LL)) = 0;
   SCOCR2LL = 0;
      
   // reset timer value
   SCCNTHH = SCCNTHL = SCCNTLH = 0;
   SCCNTLL = 0;
   
   //set period
   radiotimer_setPeriod(period);
   SCCR0 |= _BV(SCMBTS); // "reset" radiotimer
   
   // wait for register writes
   while(SCSR & 0x01);
   //enable interrupts
   SCIRQM |= 0x06;
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
   return radiotimer_getCapturedTime();
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
	SCOCR3HH = (uint8_t)(period>>24);
	SCOCR3HL = (uint8_t)(period>>16);
	SCOCR3LH = (uint8_t)(period>>8);
	SCOCR3LL = (uint8_t)period;
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
	return *((PORT_RADIOTIMER_WIDTH *)(&SCOCR3LL));
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
   // offset when to fire
	SCOCR2HH = (uint8_t)(offset>>24);
	SCOCR2HL = (uint8_t)(offset>>16);
	SCOCR2LH = (uint8_t)(offset>>8);
	SCOCR2LL = (uint8_t)offset;
   
   // reset pending interrupts
   SCIRQS |= _BV(1);   
   // enable interrupt
   SCIRQM |= _BV(1);
}

void radiotimer_cancel() {
   // reset value
   *((PORT_RADIOTIMER_WIDTH *)(&SCOCR2LL)) = 0;
   SCOCR2LL = 0;
   
   // disable interrupt
   SCIRQM &= ~_BV(1);
}

//===== capture

inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
   return *((PORT_RADIOTIMER_WIDTH *)(&SCCNTLL)) - *((PORT_RADIOTIMER_WIDTH *)(&SCBTSRLL));
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

uint8_t radiotimer_isr() {
	while(1);
}

uint8_t radiotimer_compare_isr() {
         if (radiotimer_vars.compare_cb!=NULL) {
	         // call the callback
	         radiotimer_vars.compare_cb();
	         // kick the OS
	         return 1;
         }	
		 return 0;
}

uint8_t radiotimer_overflow_isr() {
		SCCR0 |= _BV(SCMBTS);
         if (radiotimer_vars.overflow_cb!=NULL) {
	         // call the callback
	         radiotimer_vars.overflow_cb();
	         // kick the OS
	         return 1;
         }	
		 return 0;
}