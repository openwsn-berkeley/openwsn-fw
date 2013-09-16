/**
\brief LPCXpresso-specific definition of the "radiotimer" bsp module.
this timer uses 2 match (compare) registers and 1 capture register. The time counter is a free counter. The match register 0 is used
to control time slot size, it is re-scheduled after every match.
The second match register is used to schedule events within the slot.
The capture register is used to capture counter value at different moments, e.g the first bit of a packet leaves the radio.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/


#include "radiotimer.h"
#include "timer.h"
#include "LPC17xx.h"
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

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
   // source ACLK from 32kHz crystal
 //user bsp_timer.
	 timer_init(TIMER_NUM3);
	 timer_enable(TIMER_NUM3);
	 LPC_PINCON->PINSEL1      |= 0x3<<14;      // CAP3.0 mode
	// LPC_GPIO0->FIODIR        |=  1<<23;       // set as output
	// LPC_GPIO0->FIOCLR        |=  1<<23;       // set to 0

	 timer_set_capture(TIMER_NUM3,TIMER_CAPTURE_REG0);//configures capture register so that when pin cap3.0 is toggled a capture is triggered (raising edge)- (cap3.0 is in)
	 radiotimer_setPeriod(period);
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
   return timer_get_current_value(TIMER_NUM3);
}
//period is in ms???

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
	radiotimer_vars.period=(PORT_RADIOTIMER_WIDTH)period;
	timer_reset(TIMER_NUM3);
	timer_enable(TIMER_NUM3);
	radiotimer_vars.counter_slot_val=radiotimer_getValue(); //it is 0 always. remove that..
	timer_set_compare(TIMER_NUM3, TIMER_COMPARE_REG0,  radiotimer_vars.counter_slot_val+period); //the period timer is controlled by the compare 0 register
}




PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
   return (PORT_RADIOTIMER_WIDTH)radiotimer_vars.period;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
	uint32_t cur;
	PORT_RADIOTIMER_WIDTH current=radiotimer_vars.counter_slot_val;//references to the init of the current time slot.
	// offset when to fire
	//get current
	cur=current + offset;
	timer_set_compare(TIMER_NUM3, TIMER_COMPARE_REG1,cur); //this is controlled by the compare 1 register

}

void radiotimer_cancel() {
   // reset the timer.
	timer_reset_compare(TIMER_NUM3,TIMER_COMPARE_REG1);//cancel any pending timer driven by reg1 (not by the period compare register)

}

//===== capture

inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
	PORT_RADIOTIMER_WIDTH wi;
	PORT_RADIOTIMER_WIDTH wa;
	wa=timer_get_capture_value(TIMER_NUM3,TIMER_CAPTURE_REG0);
	wi= wa-radiotimer_vars.counter_slot_val;//w.r.t init of the super slot -- as timer is reset now, counter val is 0 always.
	return wi;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

void private_radio_timer_isr(uint8_t source) {
	uint32_t current=0;
	switch (source) {
	case TIMER_COMPARE_REG0:
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
