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

//=========================== defines =========================================

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
//void   abstimer_reschedule_in_period();

//=========================== public ==========================================

//===== from bsp_timer
void bsp_timer_init() {
   abstimer_init();
}

void bsp_timer_set_callback(bsp_timer_cbt cb) {
   abstimer_vars.callback[ABSTIMER_SRC_BSP_TIMER] = cb;
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
   abstimer_vars.bsp_timer_total=delayTicks;
   
   // set the compare value (since last one)
   abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER]   += delayTicks;
   
   // I'm using this timer
   abstimer_vars.isArmed[ABSTIMER_SRC_BSP_TIMER]  = TRUE;
   
   // reschedule
   abstimer_reschedule();
}

/**
 * cancels the bsp timer. 
 * sets it to not running and schedules any other running timer
 */
void bsp_timer_cancel_schedule() {
   //clear total tics.
   abstimer_vars.bsp_timer_total=0;	
    
   //clear the compare value   
   abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER]   = 0;
	   
   // I'm not using this timer
   abstimer_vars.isArmed[ABSTIMER_SRC_BSP_TIMER]  = FALSE;
   
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
   abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_OVERFLOW] = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_COMPARE] = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   abstimer_vars.radiotimer_startFrame_cb = cb;
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   abstimer_vars.radiotimer_endFrame_cb = cb;
}

void radiotimer_start(uint16_t period) {
   
   // remember the period
   abstimer_vars.radiotimer_period                            = period;

   // remember previous overflow value
   abstimer_vars.radiotimer_overflow_previousVal              = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];

   // update the timer value (calculated as one period since the last one)
   abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]   += abstimer_vars.radiotimer_period;
   
   // I'm using this timer
   abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]       = TRUE;
   
   // reschedule
   abstimer_reschedule();
}

uint16_t radiotimer_getValue() {
   return 0;
   // TODO
}

void radiotimer_setPeriod(uint16_t period) {
   abstimer_vars.radiotimer_period=period;
   
   //what to do here? -- the set period can be set in any moment, even in a middle of n execution.
   //do we want to replace the current one or in contrast just set the new compare value relative to the previous one?
   
   //keep previous value == init time for this timer.
   abstimer_vars.radiotimer_overflow_previousVal=abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
   
   //set the timeout in the future.
   abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW] += abstimer_vars.radiotimer_period;
   
   // I'm using this timer
   abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]     = TRUE;
   
   // reschedule
   abstimer_reschedule();
}

uint16_t radiotimer_getPeriod() {
   return abstimer_vars.radiotimer_period;
}

void radiotimer_schedule(uint16_t offset) {
  
   // remember the offset
   abstimer_vars.radiotimer_compare_offset                    = offset;
   
   // set the compare value since previous *overflow*
   abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]     = abstimer_vars.radiotimer_overflow_previousVal + \
                                                             abstimer_vars.radiotimer_compare_offset;
   
   // I'm using this timer
   abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE]        = TRUE;
   
   // reschedule
   abstimer_reschedule();
}

/**
 * cancels the compare timer, not the period of the radiotimer.
 * sets it to not running and reschedules.
 **/
void radiotimer_cancel() {
   abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE]     = FALSE;

   abstimer_reschedule();
}

/**
  think on that. this has to return the current value of the  radiotimer compare timer 
  that is relative to the period timer.
**/
uint16_t radiotimer_getCapturedTime() {
   return 0;
   // TODO
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
      
      // declare as initialized
      abstimer_vars.initialized = TRUE;
   }
}

//===== rescheduling
//void abstimer_reschedule_in_period() {
//   // the goal here is to take the next compare time,
//   // i.e. the one with the smallest difference to currentTime which is armed
//   uint16_t valToLoad;  // the value to eventually load in the compare register
//   uint16_t now;
//  
//   now = sctimer_getValue();
//   
//   // find the timer closest in time to now
//   if (abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE]==TRUE) {
//        if( abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]-now<abstimer_vars.currentTime - now){//this timeout is earlier than current timeout.
//            valToLoad              = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE];
//            abstimer_vars.nextToFire  = ABSTIMER_SRC_RADIOTIMER_COMPARE;
//            sctimer_schedule(valToLoad);
//            abstimer_vars.currentTime = valToLoad;
//         }
//      }
//}

uint16_t abstimer_reschedule() {
   // the goal here is to take the next compare time,
   // i.e. the one with the smallest difference to currentTime which is armed
   
   uint8_t  i;          // iterator
   bool     found;      // TRUE iff a timer could be found to schedule
   uint16_t valToLoad;  // the value to eventually load in the compare register
   uint16_t thisDist;   // the distance in the future of this timer
   uint16_t minDist;    // the minimem distance amoung all timers
  
  // uint16_t now;
  // now = sctimer_getValue();
   
   minDist = 0xffff;
   
   // find the timer closest in time to now
   found = FALSE;
   for (i=0;i<ABSTIMER_SRC_MAX;i++) {
      if (abstimer_vars.isArmed[i]==TRUE) {
        //attention! this is not correct, whenever a radiocompare is set, this does not work.
        // the radio compare is scheduled always before the end of the period. currenttime is
        //represents the end of the period so in this case the distance will be a complete wrap of
        // the timer. so will be scheduled the last one when probably is the next one to expire.
        thisDist = abstimer_vars.compareVal[i]-abstimer_vars.currentTime;
         if (
                found==FALSE ||
                thisDist<minDist
             ) {
            valToLoad              = abstimer_vars.compareVal[i];
            minDist                = thisDist;
            found                  = TRUE;
            abstimer_vars.nextToFire  = (abstimer_src_t)i;
         }
      }
   }
   
   // load that timer, if found
   if (found==TRUE) {
      sctimer_schedule(valToLoad);
      abstimer_vars.nextCurrentTime = valToLoad;
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
   uint16_t        startTime;               // the value of the timer when this function starts running
   uint16_t        timeToNextInterrupt;
   uint8_t         bitmapInterruptsFired;
   
   // remember what time it is now
   startTime                 = sctimer_getValue();
   
   // update the current theoretical time
   abstimer_vars.currentTime = abstimer_vars.nextCurrentTime;
   
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
      abstimer_vars.callback[ABSTIMER_SRC_BSP_TIMER]();
   }
   if (bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_OVERFLOW)) {
      abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]();
   }
   if (bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_COMPARE)) {
      abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_COMPARE]();
   }
   
   //===== step 4. schedule the next operation
   
   timeToNextInterrupt       = abstimer_reschedule();
   
   //===== step 5. make sure I'm not late for my next schedule
   while ((sctimer_getValue()-startTime)>timeToNextInterrupt) {
      // I've already passed the interrupt I was supposed to schedule
      
      // update debug statistics
      abstimer_dbg.num_late_schedule++;
   }
   
   // kick the OS
   return 1;
}