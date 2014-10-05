///**
// \brief A BSP module which abstracts away the "bsp_timer" and "radiotimer"
// modules behind the "sctimer".
//
// \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
// \author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2012.
// */
//
//#include "opendefs.h"
//#include "bsp_timer.h"
//#include "radiotimer.h"
//#include "sctimer.h"
//#include "debugpins.h"
//
////=========================== defines =========================================
//
//#define ABSTIMER_GUARD_TICKS 3
//
//typedef void (*abstimer_cbt)();
//
////=========================== variables =======================================
//
//typedef enum {
//	ABSTIMER_SRC_BSP_TIMER = 0,
//	ABSTIMER_SRC_RADIOTIMER_OVERFLOW,
//	ABSTIMER_SRC_RADIOTIMER_COMPARE,
//	ABSTIMER_SRC_MAX,
//} abstimer_src_t;
//
//typedef struct {
//	uint16_t num_bsp_timer;
//	uint16_t num_radiotimer_overflow;
//	uint16_t num_radiotimer_compare;
//	uint16_t num_late_schedule;
//	uint16_t isr_time;
//	uint16_t isr_time_last;
//	abstimer_cbt max_callback[ABSTIMER_SRC_MAX];
//	uint16_t isr_max_times;
//	uint16_t isr_late_max;
//} abstimer_dbg_t;
//
//abstimer_dbg_t abstimer_dbg;
//
//typedef struct {
//	// admin
//	bool initialized;
//	uint16_t currentTime; // current "theoretical" time
//	uint16_t nextCurrentTime; // next "theoretical" time
//	// callbacks
//	radiotimer_capture_cbt radiotimer_startFrame_cb;
//	radiotimer_capture_cbt radiotimer_endFrame_cb;
//	// timer values
//	abstimer_cbt callback[ABSTIMER_SRC_MAX];
//	bool isArmed[ABSTIMER_SRC_MAX]; //if they are armed.
//	uint16_t compareVal[ABSTIMER_SRC_MAX];//value on the compare register
//	bool nextToFire[ABSTIMER_SRC_MAX]; //true if is the next to be fired. can be more than one.
//	// radiotimers-specific variables
//	uint16_t radiotimer_period;
//	uint16_t radiotimer_overflow_previousVal;//time of init of the period
//	uint16_t radiotimer_compare_offset; //w.r.t the init of the period
//	// bsp-timer-specific variables
//	uint16_t bsp_timer_total; // total time of the scheduled bsp timer (relative value)
//} abstimer_vars_t;
//
//abstimer_vars_t abstimer_vars;
//
////=========================== prototypes ======================================
//
//void abstimer_init();
//uint16_t abstimer_reschedule();
//void sctimer_init();
//void sctimer_schedule(uint16_t val);
//uint16_t sctimer_getValue();
//
////=========================== public ==========================================
//
////===== from bsp_timer
//void bsp_timer_init() {
//	abstimer_init();
//}
//
//void bsp_timer_set_callback(bsp_timer_cbt cb) {
//	abstimer_vars.callback[ABSTIMER_SRC_BSP_TIMER] = cb;
//}
//
///*
// * clear the hardware timer and the current data structures. So resets everything.
// */
//void bsp_timer_reset() {
//	bsp_timer_cancel_schedule();
//}
//
///**
// * schedules the next bsp timer in delayTics.
// */
//void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks) {
//	//keep tics
//	abstimer_vars.bsp_timer_total = delayTicks;
//
//	// set the compare value (since last one)
//	abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER] += delayTicks;
//
//	// I'm using this timer
//	abstimer_vars.isArmed[ABSTIMER_SRC_BSP_TIMER] = TRUE;
//
//	// reschedule
//	abstimer_reschedule();
//}
//
///**
// * cancels the bsp timer. 
// * sets it to not running and schedules any other running timer
// */
//void bsp_timer_cancel_schedule() {
//	//clear total tics.
//	abstimer_vars.bsp_timer_total = 0;
//
//	//clear the compare value   
//	abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER] = 0;
//
//	// I'm not using this timer
//	abstimer_vars.isArmed[ABSTIMER_SRC_BSP_TIMER] = FALSE;
//
//	// reschedule
//	abstimer_reschedule();
//}
//
///**
// * timers are relative to the last compare 
// * the elapsed time should be total -(compareVal-current)
// * 
// */
//PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
//	return abstimer_vars.bsp_timer_total
//			- (abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER]
//			                            - sctimer_getValue());
//}
//
////===== from radiotimer
//void radiotimer_init() {
//	abstimer_init();
//}
//
//void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
//	abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_OVERFLOW] = cb;
//}
//
//void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
//	abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_COMPARE] = cb;
//}
//
//void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
//	abstimer_vars.radiotimer_startFrame_cb = cb;
//}
//
//void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
//	abstimer_vars.radiotimer_endFrame_cb = cb;
//}
//
//void radiotimer_start(uint16_t period) {
//
//	// remember the period
//	abstimer_vars.radiotimer_period = period;
//
//	// remember previous overflow value
//	abstimer_vars.radiotimer_overflow_previousVal
//	= abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
//
//	// update the timer value (calculated as one period since the last one)
//	abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]
//	                         += abstimer_vars.radiotimer_period;
//
//	// I'm using this timer
//	abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_OVERFLOW] = TRUE;
//
//	// reschedule
//	abstimer_reschedule();
//}
//
////this is the elapsed time in this period (now - previous val)  
//uint16_t radiotimer_getValue() {
//	return sctimer_getValue() - abstimer_vars.radiotimer_overflow_previousVal;
//}
//
//void radiotimer_setPeriod(uint16_t period) {
//
//	uint16_t oldperiod = abstimer_vars.radiotimer_period;
//
//	abstimer_vars.radiotimer_period = period;
//
//	//correct the period in that current slot. so remove the current period
//	abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW] -= oldperiod;
//
//	//keep init time          
//	abstimer_vars.radiotimer_overflow_previousVal = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
//
//	//set new period
//	abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]
//	                         += abstimer_vars.radiotimer_period;
//
//	// reschedule
//	abstimer_reschedule();
//}
//
//uint16_t radiotimer_getPeriod() {
//	return abstimer_vars.radiotimer_period;
//}
//
//void radiotimer_schedule(uint16_t offset) {
//	// remember the offset
//	abstimer_vars.radiotimer_compare_offset = offset;
//
//	// set the compare value since previous *overflow*
//	abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]
//	                         = abstimer_vars.radiotimer_overflow_previousVal
//	                         + abstimer_vars.radiotimer_compare_offset;
//
//	// I'm using this timer
//	abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE] = TRUE;
//
//	// reschedule
//	abstimer_reschedule();
//}
//
//void radiotimer_cancel() {
//	abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE] = FALSE;
//
//	abstimer_reschedule();
//}
//
////the current value as we do not have a capture register.
//uint16_t radiotimer_getCapturedTime() {
//	return radiotimer_getValue();
//}
//
////=========================== private =========================================
//
////===== admin
//void abstimer_init() {
//	// only initialize once
//	if (abstimer_vars.initialized == FALSE) {
//		// clear module variables
//		memset(&abstimer_vars, 0, sizeof(abstimer_vars_t));
//
//		// start the HW timer
//		sctimer_init();
//
//		//set callback in case the hardware timer needs it. IAR based projects use pragma to bind it.
//
//		sctimer_setCb(radiotimer_isr);
//
//		// declare as initialized
//		abstimer_vars.initialized = TRUE;
//	}
//}
//
////===== rescheduling
//
//uint16_t abstimer_reschedule() {
//	// the goal here is to take the next compare time,
//	// i.e. the one with the smallest difference to currentTime which is armed
//
//	uint8_t i, j; // iterator
//	bool found; // TRUE iff a timer could be found to schedule
//	uint16_t valToLoad; // the value to eventually load in the compare register
//	uint16_t thisDist; // the distance in the future of this timer
//	uint16_t minDist; // the minimem distance amoung all timers
//
//	minDist = 0xffff;
//
//	// find the timer closest in time to now
//	found = FALSE;
//
//	for (i = 0; i < ABSTIMER_SRC_MAX; i++) {
//		if (abstimer_vars.isArmed[i] == TRUE) {
//			//calculate the distance
//			thisDist = abstimer_vars.compareVal[i] - abstimer_vars.currentTime;
//			if (found == FALSE || thisDist < minDist) {
//				valToLoad = abstimer_vars.compareVal[i];
//				minDist = thisDist;
//				found = TRUE;
//			}
//		}
//	}
//
//	// load that timer, if found
//	if (found == TRUE) {
//		sctimer_schedule(valToLoad);
//		abstimer_vars.nextCurrentTime = valToLoad;
//		//set all those that will expire next.
//		for (i = 0; i < ABSTIMER_SRC_MAX; i++) {
//			if (abstimer_vars.compareVal[i] == valToLoad
//					&& abstimer_vars.isArmed[i] == TRUE) {
//				abstimer_vars.nextToFire[i] = TRUE;
//			} else {
//				abstimer_vars.nextToFire[i] = FALSE;
//			}
//		}
//	} else {
//		while (1);
//	}
//
//	// return in how long the timer will fire
//	return minDist;
//}
//
////=========================== interrupts ======================================
//
//uint8_t bsp_timer_isr() {
//	// the lptmr module uses only TimerA, so this should never happen
//	while (1);
//}
//
//uint8_t radiotimer_isr() {
//	volatile uint8_t i, j, min_exec; // iterator
//	volatile uint16_t now, timeSpent,dbg_init_time,dbg_end_time;
//	volatile uint16_t calc,max,max2;
//	volatile uint16_t min;
//	volatile bool late[ABSTIMER_SRC_MAX]; //all that are late
//	volatile uint16_t distance[ABSTIMER_SRC_MAX]; //the distance to the current time when they are late
//	volatile bool exists, end;
//	volatile bool tobefired[ABSTIMER_SRC_MAX];//those that need to be fired now.
//	volatile abstimer_cbt currentcall[ABSTIMER_SRC_MAX];
//	
//	max=0;
//	max2=0;
//	sctimer_clearISR();
//	debugpins_isr_toggle();
//
//	now = sctimer_getValue();
//    
//	// update the current theoretical time
//	abstimer_vars.currentTime = abstimer_vars.nextCurrentTime;
//
//	for (i = 0; i < ABSTIMER_SRC_MAX; i++) {
//		tobefired[i] = FALSE;
//	}
//
//	//for each timer that expired check wether needs to be rescheduled.
//	for (i = 0; i < ABSTIMER_SRC_MAX; i++) {
//		if ((abstimer_vars.isArmed[i] == TRUE) && (abstimer_vars.nextToFire[i]
//		                                                                    == TRUE)) {
//			switch (i) {
//			case ABSTIMER_SRC_BSP_TIMER:
//				abstimer_dbg.num_bsp_timer++;
//				break;
//			case ABSTIMER_SRC_RADIOTIMER_OVERFLOW:
//				abstimer_dbg.num_radiotimer_overflow++;
//				//keep previous value
//				abstimer_vars.radiotimer_overflow_previousVal = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
//				// this is a periodic timer, reschedule automatically
//				abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]+= abstimer_vars.radiotimer_period;
//				break;
//			case ABSTIMER_SRC_RADIOTIMER_COMPARE:
//				abstimer_dbg.num_radiotimer_compare++;
//				// reschedule automatically after *overflow*
//				abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]= abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]
//				                                                    + abstimer_vars.radiotimer_compare_offset;
//				break;
//			}
//			tobefired[i] = TRUE;//set to be fired.
//		}
//	}
//
//	//===== step 3. call the callbacks of the timers that just fired
//
//
//	for (i = 0; i < ABSTIMER_SRC_MAX; i++) {
//		if ((abstimer_vars.isArmed[i] == TRUE) && (tobefired[i] == TRUE)) {
//			abstimer_vars.nextToFire[i] = FALSE; //fired.
//			abstimer_vars.callback[i]();
//			currentcall[i]=abstimer_vars.callback[i];
//		}else{
//			currentcall[i]=0;
//		}
//	}
//
//	//===== step 4. schedule the next operation
//
//	abstimer_reschedule();
//
//	//===== step 5. make sure I'm not late for my next schedule
//
//	// evaluate how much time has passed since theoretical currentTime,
//	// (over estimate by ABSTIMER_GUARD_TICKS)
//	//now = sctimer_getValue(); 
//	//timeSpent =(uint16_t)now - (uint16_t)abstimer_vars.currentTime;   
//	//timeSpent += ABSTIMER_GUARD_TICKS;
//
//	timeSpent = sctimer_getValue();
//	timeSpent-= now; //this is the real elapsed time.
//	//	timeSpent =(uint16_t)now - (uint16_t)abstimer_vars.currentTime;   
//	timeSpent += ABSTIMER_GUARD_TICKS;
//
//	//find smallest next timer that is already late. aka compareVal - time at the beginning > timeElapsed. 
//
//	for (i=0;i<ABSTIMER_SRC_MAX;i++) {
//		//calculate distance to next timeout
//		calc=(uint16_t)abstimer_vars.compareVal[i] - (uint16_t)now; 	
//		if ((abstimer_vars.isArmed[i]==TRUE) &&	(timeSpent>=calc)) {
//			// this interrupt needs to be serviced now
//			late[i]=TRUE;
//			distance[i]=calc;
//			// update debug statistics
//			abstimer_dbg.num_late_schedule++;
//		}else{
//			late[i]=FALSE;
//			distance[i]=0xFFFF;
//		}
//	}
//
//	//execute late timers in order
//	end=FALSE;
//	while (end==FALSE) {
//		//find all mins
//		min=0xFFFF;
//		end=TRUE;
//		for (j=0;j<ABSTIMER_SRC_MAX;j++){
//			if ((abstimer_vars.isArmed[j]==TRUE) && (late[j]==TRUE)) {
//				if (distance[j]<min){
//					min=distance[j];
//					min_exec=j;
//					exists=TRUE;
//					end&=FALSE;
//				}else{
//					//if all are true
//					end&=TRUE;
//				}
//			}
//		}
//		//call callback in order
//		if (exists==TRUE){
//			dbg_init_time=sctimer_getValue();
//			abstimer_vars.nextCurrentTime=abstimer_vars.compareVal[min_exec];
//			abstimer_vars.currentTime = abstimer_vars.nextCurrentTime;
//			abstimer_vars.nextToFire[min_exec] = FALSE;
//			abstimer_vars.callback[min_exec]();
//			late[min_exec]=FALSE;
//			exists=FALSE;
//			abstimer_reschedule();
//			dbg_end_time=sctimer_getValue();
//			
//
//			max2=(uint16_t)dbg_end_time-(uint16_t)dbg_init_time;
//			if (max2>=abstimer_dbg.isr_late_max){
//				abstimer_dbg.isr_late_max=max2;
//			}
//			
//			//TODO .. this execution may take a lot of time so maybe this makes some other timers to be missed.
//		}
//	}
//
//	//reschedule as callbacks may be reset some timers.
//	//abstimer_reschedule();
//
//	//get maximum isr time to see if the guard time is enough or to big.
//	max=((uint16_t)sctimer_getValue()-(uint16_t)now);
//	abstimer_dbg.isr_time_last=max;//debug.
//	
//	if (max>=abstimer_dbg.isr_time){
//		abstimer_dbg.isr_time=max;//debug.
//		abstimer_dbg.isr_max_times++;//debug.
//		for (i=0;i<ABSTIMER_SRC_MAX;i++){
//			abstimer_dbg.max_callback[i]=currentcall[i];
//		}
//	}
//	// kick the OS
//	return 1;
//}