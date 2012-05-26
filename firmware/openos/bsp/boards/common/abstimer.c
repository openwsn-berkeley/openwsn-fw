/**
\brief A BSP module which abstracts away the "bsp_timer" and "radiotimer"
       modules behind the "sctimer".

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2012.
 */

#include "openwsn.h"
#include "bsp_timer.h"
#include "radiotimer.h"
#include "sctimer.h"
#include "debugpins.h"

//=========================== defines =========================================

#define ABSTIMER_GUARD_TICKS 3

typedef void (*abstimer_cbt)(void);

//=========================== variables =======================================

typedef enum {
	ABSTIMER_SRC_BSP_TIMER = 0,
	ABSTIMER_SRC_RADIOTIMER_OVERFLOW,
	ABSTIMER_SRC_RADIOTIMER_COMPARE,
	ABSTIMER_SRC_MAX,
} abstimer_src_t;

typedef struct {
	uint16_t                  num_bsp_timer;
	uint16_t                  num_radiotimer_overflow;
	uint16_t                  num_radiotimer_compare;
	uint16_t                  num_late_schedule;
} abstimer_dbg_t;

abstimer_dbg_t abstimer_dbg;

typedef struct {
	// admin
	bool                      initialized;
	uint16_t                  currentTime;        // current "theoretical" time
	uint16_t                  nextCurrentTime;    // next "theoretical" time
	// callbacks
	radiotimer_capture_cbt    radiotimer_startFrame_cb;
	radiotimer_capture_cbt    radiotimer_endFrame_cb;
	// timer values
	abstimer_cbt              callback[ABSTIMER_SRC_MAX];
	bool                      isArmed[ABSTIMER_SRC_MAX];
	uint16_t                  compareVal[ABSTIMER_SRC_MAX];
	abstimer_src_t            nextToFire;
	// radiotimers-specific variables
	uint16_t                  radiotimer_period;
	uint16_t                  radiotimer_overflow_previousVal;
	uint16_t                  radiotimer_compare_offset;
	// bsp-timer-specific variables
	uint16_t                  bsp_timer_total;    // total time of the scheduled bsp timer (relative value)
} abstimer_vars_t;

abstimer_vars_t abstimer_vars;


//=========================== prototypes ======================================

void     abstimer_init();
uint16_t abstimer_reschedule();
void     sctimer_init();
void     sctimer_schedule(uint16_t val);
uint16_t sctimer_getValue();

//=========================== public ==========================================

//===== from bsp_timer
void bsp_timer_init() {
	abstimer_init();
}

void bsp_timer_set_callback(bsp_timer_cbt cb) {
	abstimer_vars.callback[ABSTIMER_SRC_BSP_TIMER]               = cb;
}

/*
 * clear the hardware timer and the current data structures. So resets everything.
 */
void bsp_timer_reset() {
	bsp_timer_cancel_schedule();
}

/**
 * schedules the next bsp timer in delayTics.
 */
void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks) {
	//keep tics
	abstimer_vars.bsp_timer_total                                = delayTicks;

	// set the compare value (since last one)
	abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER]            += delayTicks;

	// I'm using this timer
	abstimer_vars.isArmed[ABSTIMER_SRC_BSP_TIMER]                = TRUE;

	// reschedule
	abstimer_reschedule();
}

/**
 * cancels the bsp timer. 
 * sets it to not running and schedules any other running timer
 */
void bsp_timer_cancel_schedule() {
	//clear total tics.
	abstimer_vars.bsp_timer_total                                = 0;

	//clear the compare value   
	abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER]             = 0;

	// I'm not using this timer
	abstimer_vars.isArmed[ABSTIMER_SRC_BSP_TIMER]                = FALSE;

	// reschedule
	abstimer_reschedule();
}

/**
 * timers are relative to the last compare 
 * the elapsed time should be total -(compareVal-current)
 * 
 */
PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
	return abstimer_vars.bsp_timer_total - (abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER] - sctimer_getValue());
}

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

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
	abstimer_vars.radiotimer_startFrame_cb                       = cb;
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
	abstimer_vars.radiotimer_endFrame_cb                         = cb;
}

void radiotimer_start(uint16_t period) {

	// remember the period
	abstimer_vars.radiotimer_period                              = period;

	// remember previous overflow value
	abstimer_vars.radiotimer_overflow_previousVal                = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];

	// update the timer value (calculated as one period since the last one)
	abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]  += abstimer_vars.radiotimer_period;

	// I'm using this timer
	abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]      = TRUE;

	// reschedule
	abstimer_reschedule();
}

//this is the elapsed time in this period (now - previous val)  
uint16_t radiotimer_getValue() {
	return sctimer_getValue()-abstimer_vars.radiotimer_overflow_previousVal;
}

void radiotimer_setPeriod(uint16_t period) {

	uint16_t oldperiod=abstimer_vars.radiotimer_period;

	abstimer_vars.radiotimer_period=period;

        //correct the period in that current slot. so remove the current period
	abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]  -= oldperiod;

        //keep init time          
        abstimer_vars.radiotimer_overflow_previousVal=abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
	//set new period
        abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]  += abstimer_vars.radiotimer_period;

	// reschedule
	abstimer_reschedule();
}

uint16_t radiotimer_getPeriod() {
	return abstimer_vars.radiotimer_period; 
}

void radiotimer_schedule(uint16_t offset) {
       // remember the offset
	abstimer_vars.radiotimer_compare_offset                      = offset;
       
	// set the compare value since previous *overflow*
	abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]    = abstimer_vars.radiotimer_overflow_previousVal + \
			abstimer_vars.radiotimer_compare_offset;
          
	// I'm using this timer
	abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE]       = TRUE;

	// reschedule
	abstimer_reschedule();
}


void radiotimer_cancel() {
	abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE]       = FALSE;

	abstimer_reschedule();
}

//the current value as we do not have a capture register.
uint16_t radiotimer_getCapturedTime() {
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

		//set callback in case the hardware timer needs it. IAR based projects use pragma to bind it.

		sctimer_setCb(radiotimer_isr);

		// declare as initialized
		abstimer_vars.initialized = TRUE;
	}
}

//===== rescheduling

uint16_t abstimer_reschedule() {
	// the goal here is to take the next compare time,
	// i.e. the one with the smallest difference to currentTime which is armed

	uint8_t  i;          // iterator
	bool     found;      // TRUE iff a timer could be found to schedule
	uint16_t valToLoad;  // the value to eventually load in the compare register
	uint16_t thisDist;   // the distance in the future of this timer
	uint16_t minDist;    // the minimem distance amoung all timers

	minDist = 0xffff;

	// find the timer closest in time to now
	found = FALSE;
	for (i=0;i<ABSTIMER_SRC_MAX;i++) {
		if (abstimer_vars.isArmed[i]==TRUE) {
			thisDist = abstimer_vars.compareVal[i]-abstimer_vars.currentTime;
			if (
					found==FALSE ||
					thisDist<minDist
			) {
				valToLoad                  = abstimer_vars.compareVal[i];
				minDist                    = thisDist;
				found                      = TRUE;
				abstimer_vars.nextToFire   = (abstimer_src_t)i;
			}
		}
	}

	// load that timer, if found
	if (found==TRUE) {
		sctimer_schedule(valToLoad);
		abstimer_vars.nextCurrentTime    = valToLoad;
	}else{
            while(1);
        }

	// return in how long the timer will fire
	return minDist;
}

//=========================== interrupts ======================================

uint8_t bsp_timer_isr() {
	// the lptmr module uses only TimerA, so this should never happen
	while(1);
}

uint8_t radiotimer_isr() {
	uint8_t         i;                       // iterator
	uint16_t        timeSpent,timeSpent2;
	uint8_t         bitmapInterruptsFired;
	uint16_t        calc,real_counter_val_new;
	bool            update;
	uint16_t        min;

        sctimer_clearISR();
        
        debugpins_isr_toggle();
	
        // update the current theoretical time
	real_counter_val_new  =sctimer_getValue();

	abstimer_vars.currentTime = abstimer_vars.nextCurrentTime;


	if (real_counter_val_new<abstimer_vars.currentTime) {
		while(1);
	}

	//===== step 1. Find out which interrupts just fired

	// Note: we store the list of interrupts which fired in an 8-bit bitmap
	bitmapInterruptsFired = 0;
	
        for (i=0;i<ABSTIMER_SRC_MAX;i++) {
		if (
				(abstimer_vars.isArmed[i]==TRUE) &&
				(abstimer_vars.compareVal[i]==abstimer_vars.currentTime)
		) {
			bitmapInterruptsFired |= (1<<i);
        	}
	}

	// make sure at least one timer fired
	if (bitmapInterruptsFired==0) {
          while(1); 
	}

	while (bitmapInterruptsFired!=0) {
		
	//==== step 2. Reschedule the timers which fired and need to be rescheduled

		if (bitmapInterruptsFired & (1<<ABSTIMER_SRC_BSP_TIMER)) {
			// update debug stats
			abstimer_dbg.num_bsp_timer++;
			// no automatic rescheduling	
		}
		if (bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_OVERFLOW)) {
			// update debug stats
			abstimer_dbg.num_radiotimer_overflow++;
			//keep previous value
			abstimer_vars.radiotimer_overflow_previousVal=abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
			// this is a periodic timer, reschedule automatically
			abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW] += abstimer_vars.radiotimer_period;
        	}
		if (bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_COMPARE)) {
			// update debug stats
			abstimer_dbg.num_radiotimer_compare++;
			// reschedule automatically after *overflow*
			abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]  = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]+ \
					abstimer_vars.radiotimer_compare_offset;

		}

		//===== step 3. call the callbacks of the timers that just fired

		if (bitmapInterruptsFired & (1<<ABSTIMER_SRC_BSP_TIMER)) {
			// call the callback
			abstimer_vars.callback[ABSTIMER_SRC_BSP_TIMER]();
			// clear the interrupt flag
			bitmapInterruptsFired &= ~(1<<ABSTIMER_SRC_BSP_TIMER);
		}
		if (bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_OVERFLOW)) {
			// call the callback
			abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]();
			// clear the interrupt flag
			bitmapInterruptsFired &= ~(1<<ABSTIMER_SRC_RADIOTIMER_OVERFLOW);
		}
		if (bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_COMPARE)) {
			// call the callback
			abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_COMPARE]();
			// clear the interrupt flag
			bitmapInterruptsFired &= ~(1<<ABSTIMER_SRC_RADIOTIMER_COMPARE);
		}

		// make sure all interrupts have been handled
		if (bitmapInterruptsFired!=0) {
			while(1);
		}

		//===== step 4. schedule the next operation

		abstimer_reschedule();

		//===== step 5. make sure I'm not late for my next schedule

		// evaluate how much time has passed since theoretical currentTime,
		// (over estimate by ABSTIMER_GUARD_TICKS)
		timeSpent = sctimer_getValue(); 
		timeSpent2 =(uint16_t)timeSpent - (uint16_t)abstimer_vars.currentTime;   
		timeSpent2 += ABSTIMER_GUARD_TICKS;
	
//debug
		if (timeSpent2 >600){
			while(1);
		}
		
		// verify that, for each timer, that duration doesn't exceed how much time is left
		bitmapInterruptsFired = 0;

		min=0xFFFF;
		//find smallest next timer and mark it in the bitmap to be executed next. Can only be one marked at a time. 
		for (i=0;i<ABSTIMER_SRC_MAX;i++) {

			//calculate distance to next timeout
			calc=(uint16_t)abstimer_vars.compareVal[i]-(uint16_t)abstimer_vars.currentTime; 
			
                        if ((abstimer_vars.isArmed[i]==TRUE) &&	(timeSpent2>calc) && (calc<min)) {
				min=calc;//this is the nearest in time
				// this interrupt needs to be serviced now
				bitmapInterruptsFired = (1<<i);
				//update the next timeout value
				abstimer_vars.nextCurrentTime=abstimer_vars.compareVal[i];  
				// update debug statistics
				abstimer_dbg.num_late_schedule++;
			}
		}
		if (bitmapInterruptsFired!=0) {
                 abstimer_vars.currentTime = abstimer_vars.nextCurrentTime;   
                 if (abstimer_vars.nextToFire==ABSTIMER_SRC_RADIOTIMER_COMPARE){
                  abstimer_vars.currentTime +=0;      //to debug
                }
		}
                
	}//end loop
	// kick the OS
	return 1;
}