/**
\brief A BSP module which abstracts away the "radiotimer" modules behind the "sctimer".

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2012.
 */

#include "opendefs.h"
#include "radiotimer.h"
#include "sctimer.h"
#include "debugpins.h"

//=========================== defines =========================================

#define ABSTIMER_GUARD_TICKS 2

typedef void (*abstimer_cbt)();

enum abstimer_irqstatus_enum{
	RADIOTIMER_NONE = 0,
	RADIOTIMER_OVERFLOW,
	RADIOTIMER_COMPARE,
};
//=========================== variables =======================================

typedef enum {
   ABSTIMER_SRC_RADIOTIMER_OVERFLOW =0,
   ABSTIMER_SRC_RADIOTIMER_COMPARE,
   ABSTIMER_SRC_MAX,
} abstimer_src_t;


typedef struct {
   // admin
   bool                      initialized;
   // timer values
   abstimer_cbt              callback[ABSTIMER_SRC_MAX];
   // radiotimers-specific variables
   PORT_RADIOTIMER_WIDTH                  radiotimer_period;
   
   uint8_t                   overflowORcompare;
} abstimer_vars_t;

abstimer_vars_t abstimer_vars;


//=========================== prototypes ======================================
void     abstimer_init();

//=========================== public ==========================================

//===== from radiotimer
void radiotimer_init() {
   abstimer_init();
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]     = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_COMPARE]      = cb;
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   // remember the period
   abstimer_vars.radiotimer_period                              = period;
   sctimer_reset();//resets counter to 0.
   sctimer_schedule(period);
   abstimer_vars.overflowORcompare=RADIOTIMER_OVERFLOW;
   ENABLE_INTERRUPTS();
}

// this is the elapsed time in this period (now - previous val)  
PORT_TIMER_WIDTH radiotimer_getValue() {
   PORT_TIMER_WIDTH x;
   x=sctimer_getValue();
   return x;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   //why??
   abstimer_vars.radiotimer_period=period;
   
   sctimer_schedule(period);
   abstimer_vars.overflowORcompare=RADIOTIMER_OVERFLOW;
    
   ENABLE_INTERRUPTS();  
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
   return abstimer_vars.radiotimer_period; 
}

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   sctimer_schedule(offset);
   abstimer_vars.overflowORcompare=RADIOTIMER_COMPARE;
   ENABLE_INTERRUPTS();
}

void radiotimer_cancel() {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   sctimer_schedule(abstimer_vars.radiotimer_period);
   abstimer_vars.overflowORcompare=RADIOTIMER_OVERFLOW;
  
   ENABLE_INTERRUPTS();
}

// the current value as we do not have a capture register.
PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
   return radiotimer_getValue();
}

//=========================== private =========================================

//===== admin
void abstimer_init() {

   // only initialize once
   if (abstimer_vars.initialized==FALSE) {
      // clear module variables
      memset(&abstimer_vars,0,sizeof(abstimer_vars_t));

      // start the HW timer
      sctimer_init();

      // set callback in case the hardware timer needs it. IAR based projects use pragma to bind it.
      sctimer_setCb(radiotimer_isr);

      // declare as initialized
      abstimer_vars.initialized = TRUE;
   }
}

//=========================== interrupts ======================================

kick_scheduler_t radiotimer_isr() {
   // kick the OS
   uint8_t source=abstimer_vars.overflowORcompare;
   kick_scheduler_t ret;
   sctimer_clearISR();
   
   switch (source){
   case RADIOTIMER_COMPARE:
	   if (abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_COMPARE]!=NULL){
		   abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_COMPARE]();
		   ret=KICK_SCHEDULER;
	   }
	   break;
   case RADIOTIMER_OVERFLOW:
	   if (abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]!=NULL){
		       sctimer_reset();//resets counter to 0.
	   		   abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]();
	   		   ret=KICK_SCHEDULER;
	   }
	   break;
   case RADIOTIMER_NONE:
   default:
	   while(1);
	   break;
   }
	
   return ret;
}