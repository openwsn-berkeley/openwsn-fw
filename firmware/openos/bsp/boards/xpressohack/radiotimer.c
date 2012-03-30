/**
\brief LPCXpresso-specific definition of the "radiotimer" bsp module.
this timer uses 2 match (compare) registers and 1 capture register. The time counter is a free counter. The match register 0 is used
to control time slot size, it is re-scheduled after every match.
The second match register is used to schedule events within the slot.
The capture register is used to capture counter value at different moments, e.g the first bit of a packet leaves the radio.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/

#include "stdio.h"
#include "string.h"
#include "radiotimer.h"
#include "timer.h"

//pin 0.23 is cap0 for capture.

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
   uint16_t period;
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

   timer_set_isr_compare_hook(TIMER_NUM3,private_radio_timer_isr);
   timer_set_isr_capture_hook(TIMER_NUM3,private_radio_timer_isr);

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

void radiotimer_start(uint16_t period) {
   // source ACLK from 32kHz crystal
 //user bsp_timer.
	 timer_init(TIMER_NUM3);
	 timer_enable(TIMER_NUM3);
	 LPC_PINCON->PINSEL1      |= 0x3<<14;      // CAP3.0 mode
	 LPC_GPIO0->FIODIR        |=  1<<23;       // set as output
	 LPC_GPIO0->FIOCLR        |=  1<<23;       // set to 0

	 timer_set_capture(TIMER_NUM3,TIMER_CAPTURE_REG0);//configures capture register so that when pin cap3.0 is toggled a capture is triggered (raising edge)- (cap3.0 is in)
	 radiotimer_setPeriod(period);
}

//===== direct access

uint16_t radiotimer_getValue() {
   return timer_get_current_value(TIMER_NUM3);
}
//period is in ms???

void radiotimer_setPeriod(uint16_t period) {
	radiotimer_vars.period=period;
	radiotimer_vars.counter_slot_val=radiotimer_getValue();
	timer_set_compare(TIMER_NUM3, TIMER_COMPARE_REG0,  period); //the period timer is controlled by the compare 0 register
}
//?? why is this needed?

uint16_t radiotimer_getPeriod() {
   return radiotimer_vars.period;
}

//===== compare

void radiotimer_schedule(uint16_t offset) {
	uint32_t current=radiotimer_vars.counter_slot_val;//references to the init of the current time slot.
	// offset when to fire
	//get current
	//current=timer_get_current_value(TIMER_NUM3);
	timer_set_compare(TIMER_NUM3, TIMER_COMPARE_REG1,current + offset); //this is controlled by the compare 1 register

}

void radiotimer_cancel() {
   // reset the timer.
	timer_reset(TIMER_NUM3);

}

//===== capture

inline uint16_t radiotimer_getCapturedTime() {
   return timer_get_capture_value(TIMER_NUM3,TIMER_CAPTURE_REG0);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

void private_radio_timer_isr(uint8_t source) {
	uint32_t current=0;
	switch (source) {
	case TIMER_COMPARE_REG0:
		 current=radiotimer_getValue();
		 radiotimer_vars.counter_slot_val=current;//refresh init of the slot.
		      // continuous timer: schedule next instant
		 timer_set_compare(TIMER_NUM3,TIMER_COMPARE_REG0,current+radiotimer_vars.period);
		if (radiotimer_vars.overflow_cb != NULL) {
			// call the callback
			radiotimer_vars.overflow_cb();
			// kick the OS
			return ;
		}

		break;
	case TIMER_COMPARE_REG1:
		if (radiotimer_vars.compare_cb != NULL) {
			// call the callback
			radiotimer_vars.compare_cb();
			// kick the OS
			return ;
		}
		break;
	default:
		while (1);
	}
	return ;
}
