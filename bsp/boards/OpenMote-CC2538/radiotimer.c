/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "radiotimer" bsp module.
 */

#include <headers/hw_ints.h>
#include <headers/hw_memmap.h>
#include <headers/hw_smwdthrosc.h>
#include <headers/hw_gptimer.h>

#include "stdio.h"
#include "string.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "interrupt.h"
#include "sys_ctrl.h"
#include "sys_ctrl.h"

#include "gptimer.h"

//=========================== defines =========================================

#define RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC     ( 976 ) // 32 MHz to 32 kHz ratio
#define GPTIMER1_STALL                          ( 0x4003100C )

//=========================== variables =======================================

typedef struct {
	bool isPeriodActive;
	bool isCompareActive;
	bool isPeriodPending;
	bool isComparePending;

	PORT_RADIOTIMER_WIDTH period_value; // period  value set
	radiotimer_compare_cbt period_cb;

	PORT_RADIOTIMER_WIDTH compare_value;
	radiotimer_compare_cbt compare_cb;

	PORT_RADIOTIMER_WIDTH sleepCounterValue;
	//dbg
	PORT_RADIOTIMER_WIDTH dbgcurrent;
	PORT_RADIOTIMER_WIDTH dbgcompare;
	PORT_RADIOTIMER_WIDTH dbgdiff;

} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

void radiotimer_isr_private(void);
kick_scheduler_t radiotimer_period_isr(void);
kick_scheduler_t radiotimer_compare_isr(void);

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
	// Clear local variables
	memset(&radiotimer_vars, 0, sizeof(radiotimer_vars_t));

	// Configure the timer as a periodic 32-bit timer
	TimerConfigure(GPTIMER1_BASE, GPTIMER_CFG_A_PERIODIC_UP);

	// Stall the timer while debugging
	HWREG(GPTIMER1_STALL) |= 0x02;
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
	radiotimer_vars.period_cb = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
	radiotimer_vars.compare_cb = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
	while (1)
		;
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
	while (1)
		;
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
	PORT_RADIOTIMER_WIDTH value = 0;

	// The module is now active
	radiotimer_vars.isPeriodActive = true;
	radiotimer_vars.isPeriodPending = false;

	radiotimer_vars.isCompareActive = false;
	radiotimer_vars.isComparePending = false;

	// Scale up from 32.768 kHz to 32 MHz
	period = period * RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC;

	// Remember the period and reset the counter
	radiotimer_vars.period_value = period;

	// Set the timer compare value
	TimerLoadSet(GPTIMER1_BASE, GPTIMER_A, period);
	// clear the counter
	TimerClearCounter(GPTIMER1_BASE, GPTIMER_A);

	// Register the timer interrupt and enable it
	TimerIntRegister(GPTIMER1_BASE, GPTIMER_BOTH, radiotimer_isr_private);
	TimerIntEnable(GPTIMER1_BASE, GPTIMER_TIMA_TIMEOUT);

	// Enable the timer interrupt
	IntEnable(INT_TIMER1A);

	// Enable the timer
    TimerEnable(GPTIMER1_BASE, GPTIMER_BOTH);
}

void radiotimer_suspend(void) {
	PORT_RADIOTIMER_WIDTH value = 0;

	value =  TimerValueGet(GPTIMER1_BASE, GPTIMER_A);//see elapsed time and sum it to the sleepCorrection as this is time that has elapsed at the moment of calling suspend
	radiotimer_vars.sleepCounterValue = value;
	//keep the timeouts for wakeup
	radiotimer_vars.period_value = TimerLoadGet(GPTIMER1_BASE,GPTIMER_A);
	radiotimer_vars.compare_value = TimerMatchGet(GPTIMER1_BASE,GPTIMER_A);

	// Disable the timer interrupt
	TimerIntDisable(GPTIMER1_BASE, INT_TIMER1A);

	// Disable the timer
	TimerDisable(GPTIMER1_BASE, GPTIMER_BOTH);

	// Stall the timer while debugging
	HWREG(GPTIMER1_STALL) |= 0x02;
}

#define MARGIN 200
void radiotimer_wakeup(PORT_TIMER_WIDTH elapsed) {
    PORT_TIMER_WIDTH period, newcounter,value;

    value =  TimerValueGet(GPTIMER1_BASE, GPTIMER_A);
   // value = radiotimer_vars.sleepCounterValue;
    //it takes 93 tics from sleep to wake up.
   // if (radiotimer_vars.sleepCounterValue+MARGIN < value){
   // 	while(1);
   // }

    // Scale up from 32.768 kHz to 32 MHz

	elapsed *= RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC;
    //add to the counter the time we have been sleeping

    newcounter= value + elapsed;

    //restore the values
    TimerLoadSet(GPTIMER1_BASE, GPTIMER_A,radiotimer_vars.period_value);
   	TimerMatchSet(GPTIMER1_BASE, GPTIMER_A,radiotimer_vars.compare_value);


    if (newcounter+MARGIN > radiotimer_vars.period_value){
    	//we are too late, pend the isr
    	TimerUpdateCounter(GPTIMER1_BASE,GPTIMER_A,0);
       // leds_debug_toggle();
    	TimerIntEnable(GPTIMER1_BASE, GPTIMER_TIMA_TIMEOUT);
    	radiotimer_vars.isPeriodPending=true;
    	TimerEnable(GPTIMER1_BASE, GPTIMER_BOTH);
    	IntPendSet(INT_TIMER1A);
    	return;
    }

    if (newcounter+MARGIN > radiotimer_vars.compare_value && radiotimer_vars.isCompareActive){
    	//we are too late, pend the isr
    	TimerUpdateCounter(GPTIMER1_BASE,GPTIMER_A,newcounter);

    	TimerIntEnable(GPTIMER1_BASE, GPTIMER_TIMA_MATCH);
    	radiotimer_vars.isComparePending=true;
    	TimerEnable(GPTIMER1_BASE, GPTIMER_BOTH);
    	IntPendSet(INT_TIMER1A);
    	return;
    }

    //no too late
	TimerUpdateCounter(GPTIMER1_BASE,GPTIMER_A,newcounter);

	TimerIntEnable(GPTIMER1_BASE, GPTIMER_TIMA_TIMEOUT);

    // Enable the timer
	TimerEnable(GPTIMER1_BASE, GPTIMER_BOTH);
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue(void) {
	PORT_RADIOTIMER_WIDTH current;

	current = TimerValueGet(GPTIMER1_BASE, GPTIMER_A);
	current /= RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC;

	return current;
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod(void) {
	PORT_RADIOTIMER_WIDTH period;

	period = radiotimer_vars.period_value;
	period /= RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC;

	return period;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
	// Scale up from 32.768 kHz to 32 MHz
	period = period * RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC;

	radiotimer_vars.period_value = period;

	radiotimer_vars.isPeriodActive = true;
	radiotimer_vars.isPeriodPending = false;

	// Set the timer compare value
	TimerLoadSet(GPTIMER1_BASE, GPTIMER_A, period);

	// Register the timer interrupt and enable it
	TimerIntRegister(GPTIMER1_BASE, GPTIMER_BOTH, radiotimer_isr_private);
	TimerIntEnable(GPTIMER1_BASE, GPTIMER_TIMA_TIMEOUT);

	// Enable the timer interrupt
	IntEnable(INT_TIMER1A);

	// Enable the timer
	TimerEnable(GPTIMER1_BASE, GPTIMER_BOTH);
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
	PORT_RADIOTIMER_WIDTH periodvalue;

	offset = offset * RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC;

	radiotimer_vars.compare_value = offset;           //keep this value

	radiotimer_vars.isCompareActive = true;

	//set the compare register
	TimerMatchSet(GPTIMER1_BASE, GPTIMER_A, offset);

	// Register the timer interrupt and enable it
	TimerIntRegister(GPTIMER1_BASE, GPTIMER_BOTH, radiotimer_isr_private);
	TimerIntEnable(GPTIMER1_BASE, GPTIMER_TIMA_MATCH|GPTIMER_TIMA_TIMEOUT);

	// Enable the timer interrupt
	IntEnable(INT_TIMER1A);

	// Enable the timer
	TimerEnable(GPTIMER1_BASE, GPTIMER_BOTH);

	//dbg
	radiotimer_vars.dbgcurrent = TimerValueGet(GPTIMER1_BASE,GPTIMER_A);
	radiotimer_vars.dbgcompare = offset;
	radiotimer_vars.dbgdiff = ((radiotimer_vars.dbgcompare -radiotimer_vars.dbgcurrent )/RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC);
}


PORT_RADIOTIMER_WIDTH radiotimer_get_remainingValue(void) {
	PORT_RADIOTIMER_WIDTH result, current, remaining;
	int32_t diff;
    //how much real time remains until the next timeout?
	current = TimerValueGet(GPTIMER1_BASE, GPTIMER_A);

	if (radiotimer_vars.isCompareActive) {
		remaining = radiotimer_vars.compare_value;// this is a trick as we know compare < period (always!)
	} else {
		remaining = radiotimer_vars.period_value;
	}

	diff =  (int32_t) ((uint32_t) remaining - (uint32_t)current); //this should always be positive as current < period (always!)
	result = (uint32_t) diff;

  	result /= RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC;

	return result;
}

void radiotimer_cancel(void) {
	radiotimer_vars.isCompareActive = false;
	radiotimer_vars.compare_value   = 0;

	TimerIntDisable(GPTIMER1_BASE, GPTIMER_TIMA_MATCH);
}

//===== capture

port_INLINE PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(void) {
	return radiotimer_getValue();
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

void radiotimer_isr_private(void) {
	uint32_t status;
	debugpins_isr_set();

	// Obtain the interrupt status
	status = TimerIntStatus(GPTIMER1_BASE, true);

    // Clear the pending interrupt
	TimerIntClear(GPTIMER1_BASE, status);
	IntPendClear(INT_TIMER1A);

	// Check the timeout interrupt status
	if ((status & GPTIMER_TIMA_TIMEOUT) == GPTIMER_TIMA_TIMEOUT ||
	     radiotimer_vars.isPeriodPending) {

	    // Clear the compare pending flag
	    radiotimer_vars.isPeriodPending = false;

	    // Execute the interrupt
		radiotimer_period_isr();
	}

	// Check the compare interrupt status
	if ((status & GPTIMER_TIMA_MATCH) == GPTIMER_TIMA_MATCH ||
	     radiotimer_vars.isComparePending) {

	    // Clear the compare pending flag
	    radiotimer_vars.isComparePending = false;

		// Execute the interrupt

		radiotimer_compare_isr();
	}

	debugpins_isr_clr();
}

kick_scheduler_t radiotimer_period_isr(void) {

	if (radiotimer_vars.period_cb != NULL && radiotimer_vars.isPeriodActive) {
		debugpins_fsm_toggle();
		radiotimer_vars.period_cb();

		// Set the timer period again as we emulate periodic
		TimerLoadSet(GPTIMER1_BASE, GPTIMER_A, radiotimer_vars.period_value);

		// Register the timer interrupt and enable it
		TimerIntRegister(GPTIMER1_BASE, GPTIMER_BOTH, radiotimer_isr_private);
		TimerIntEnable(GPTIMER1_BASE, GPTIMER_TIMA_TIMEOUT);

		// Enable the timer interrupt
		IntEnable(INT_TIMER1A);

		// Enable the timer
		TimerEnable(GPTIMER1_BASE, GPTIMER_BOTH);
	}

	// Don't kick the OS
	return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t radiotimer_compare_isr(void) {

	if (radiotimer_vars.compare_cb != NULL && radiotimer_vars.isCompareActive) {
    	radiotimer_vars.compare_value   = 0;

		radiotimer_vars.isCompareActive = false;
		radiotimer_vars.compare_cb();
		debugpins_fsm_toggle();
	}

	// Don't kick the OS
	return DO_NOT_KICK_SCHEDULER;
}
