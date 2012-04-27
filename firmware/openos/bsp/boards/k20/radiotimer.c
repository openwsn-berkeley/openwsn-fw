/**
\brief K20-specific definition of the "radiotimer" bsp module. Using opentimers.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/


#include "radiotimer.h"
#include "board.h"

//pin 0.23 is cap0 for capture.

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
   uint32_t period;
   uint32_t counter_slot_val; //timer value when the slot timer is set- references to the init of the slot
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================
void private_radio_timer_isr(uint8_t source);
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

void radiotimer_start(PORT_TIMER_WIDTH period) {
 
}

//===== direct access

PORT_TIMER_WIDTH radiotimer_getValue() {
   return 0;
}
//period is in ms???

void radiotimer_setPeriod(PORT_TIMER_WIDTH period) {
	
}




PORT_TIMER_WIDTH radiotimer_getPeriod() {
   return (PORT_TIMER_WIDTH)radiotimer_vars.period;
}

//===== compare

void radiotimer_schedule(PORT_TIMER_WIDTH offset) {
	
}

void radiotimer_cancel() {
   // reset the timer.
	

}

//===== capture

inline PORT_TIMER_WIDTH radiotimer_getCapturedTime() {
	PORT_TIMER_WIDTH wi;
	PORT_TIMER_WIDTH wa;
	return wi;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

void private_radio_timer_isr(uint8_t source) {
	uint32_t current=0;
//	switch (source) {
//	case TIMER_COMPARE_REG0:
//		if (radiotimer_vars.overflow_cb != NULL) {
//			// call the callback
//			radiotimer_vars.overflow_cb();
//			// kick the OS
//			return ;
//		}
//		break;
//	case TIMER_COMPARE_REG1:
//		if (radiotimer_vars.compare_cb != NULL) {
//			// call the callback
//			radiotimer_vars.compare_cb();
//			// kick the OS
//			return ;
//		}
//		break;
//
//	default:
//		while (1);
//	}
	return ;
}
