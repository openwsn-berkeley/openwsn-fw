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
   uint16_t                  num_radiotimer_isr;
} abstimer_dbg_t;

abstimer_dbg_t abstimer_dbg;

typedef struct {
   // admin
   bool                      initialized;
   uint16_t                  currentTime;        // current "theoretical" time
   uint16_t                  nextCurrentTime;    // next "theoretical" time
   // timer values
   abstimer_cbt              callback[ABSTIMER_SRC_MAX];
   bool                      isArmed[ABSTIMER_SRC_MAX];
   uint16_t                  compareVal[ABSTIMER_SRC_MAX];
   // radiotimers-specific variables
   uint16_t                  radiotimer_period;
   uint16_t                  radiotimer_overflow_previousVal;
   uint16_t                  radiotimer_compare_offset;
   // bsp-timer-specific variables
   uint16_t                  bsp_timer_total;    // total time of the scheduled bsp timer (relative value)
   //poipio
   uint8_t  bitmapInterruptsFired;
   uint16_t  poipoiLoop;
} abstimer_vars_t;

abstimer_vars_t abstimer_vars;


//=========================== prototypes ======================================
void     abstimer_init();
void     abstimer_reschedule();
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
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   // keep ticks
   abstimer_vars.bsp_timer_total                                = delayTicks;

   // set the compare value (since last one)
   abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER]            += delayTicks;

   // I'm using this timer
   abstimer_vars.isArmed[ABSTIMER_SRC_BSP_TIMER]                = TRUE;

   // reschedule
   abstimer_reschedule();
   ENABLE_INTERRUPTS();
}

/**
 * cancels the bsp timer. 
 * sets it to not running and schedules any other running timer
 */
 void bsp_timer_cancel_schedule() {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   // clear total tics.
   abstimer_vars.bsp_timer_total                                = 0;

   // clear the compare value   
   abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER]             = 0;

   // I'm not using this timer
   abstimer_vars.isArmed[ABSTIMER_SRC_BSP_TIMER]                = FALSE;

   // reschedule
   abstimer_reschedule();
   ENABLE_INTERRUPTS();
}

/**
 * timers are relative to the last compare 
 * the elapsed time should be total -(compareVal-current)
 * 
 */
 PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
   PORT_TIMER_WIDTH x;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   x=abstimer_vars.bsp_timer_total - (abstimer_vars.compareVal[ABSTIMER_SRC_BSP_TIMER] - sctimer_getValue());
   ENABLE_INTERRUPTS();
   return x;
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

 void radiotimer_start(uint16_t period) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
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
   ENABLE_INTERRUPTS();
}

// this is the elapsed time in this period (now - previous val)  
 PORT_TIMER_WIDTH radiotimer_getValue() {
   PORT_TIMER_WIDTH x;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   x= sctimer_getValue()-abstimer_vars.radiotimer_overflow_previousVal;
   ENABLE_INTERRUPTS();
   return x;
}

 void radiotimer_setPeriod(uint16_t period) {

   uint16_t oldperiod;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   oldperiod = abstimer_vars.radiotimer_period;

   abstimer_vars.radiotimer_period=period+1;

   // correct the period in that current slot. so remove the current period
   abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]  -= oldperiod;

   // keep init time          
   //poipoiabstimer_vars.radiotimer_overflow_previousVal=abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];

   // set new period
   abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]  += abstimer_vars.radiotimer_period;

   // reschedule
   abstimer_reschedule();
   ENABLE_INTERRUPTS();
   
}

 uint16_t radiotimer_getPeriod() {
   return abstimer_vars.radiotimer_period; 
}

 void radiotimer_schedule(uint16_t offset) {
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   // remember the offset
   abstimer_vars.radiotimer_compare_offset                      = offset;
       
   // set the compare value since previous *overflow*
   abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]    = abstimer_vars.radiotimer_overflow_previousVal + \
                                                                  abstimer_vars.radiotimer_compare_offset;
          
   // I'm using this timer
   abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE]       = TRUE;

   // reschedule
   abstimer_reschedule();
   ENABLE_INTERRUPTS();
}

 void radiotimer_cancel() {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   abstimer_vars.isArmed[ABSTIMER_SRC_RADIOTIMER_COMPARE]       = FALSE;

   abstimer_reschedule();
   ENABLE_INTERRUPTS();
}

// the current value as we do not have a capture register.
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

      // set callback in case the hardware timer needs it. IAR based projects use pragma to bind it.
      sctimer_setCb(radiotimer_isr);

      // declare as initialized
      abstimer_vars.initialized = TRUE;
   }
}

//===== rescheduling

 void abstimer_reschedule() {
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
         }
      }
   }

   // load that timer, if found
   if (found==TRUE) {
      sctimer_schedule(valToLoad);
      abstimer_vars.nextCurrentTime    = valToLoad;
   } else {
      sctimer_stop();
   }
}

//=========================== interrupts ======================================

uint8_t bsp_timer_isr() {
   // the lptmr module uses only TimerA, so this should never happen
   while(1);
}

uint8_t radiotimer_isr() {
   uint8_t         i;                       // iterator
   uint16_t        timeSpent,timeSpent2;
   uint16_t        calc;
   uint16_t        min;
   uint16_t        tempcompare;

   uint16_t taiv_temp = TAIV;                    // read only once because accessing TAIV resets it
   if (taiv_temp!=2) {
      while(1);                               // this should not happen
   }
   
   sctimer_clearISR();
   
   abstimer_dbg.num_radiotimer_isr++;
   
   debugpins_radio_set();
   
   abstimer_vars.poipoiLoop=0;
   
   // update the current theoretical time
   abstimer_vars.currentTime = abstimer_vars.nextCurrentTime;
   
   //===== step 1. Find out which interrupts just fired

   // Note: we store the list of interrupts which fired in an 8-bit bitmap
   abstimer_vars.bitmapInterruptsFired = 0;
   
   for (i=0;i<ABSTIMER_SRC_MAX;i++) {
      if (
            (abstimer_vars.isArmed[i]==TRUE) &&
            (abstimer_vars.compareVal[i]==abstimer_vars.currentTime)
      ) {
         abstimer_vars.bitmapInterruptsFired |= (1<<i);
      }
   }
   if ((abstimer_vars.bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_OVERFLOW)) &&
       (abstimer_vars.bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_COMPARE))) {
      abstimer_vars.bitmapInterruptsFired &= ~(1<<ABSTIMER_SRC_RADIOTIMER_COMPARE);
   }

   // make sure at least one timer fired
   if (abstimer_vars.bitmapInterruptsFired==0) {
      while(1);
   }
   
   while (abstimer_vars.bitmapInterruptsFired!=0) {
      
      abstimer_vars.poipoiLoop++;
      
      while (abstimer_vars.bitmapInterruptsFired!=0) {
         if       (abstimer_vars.bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_OVERFLOW)) {
            
            // update debug stats
            abstimer_dbg.num_radiotimer_overflow++;
            
            // keep previous value poipoi
            abstimer_vars.radiotimer_overflow_previousVal = \
                          abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
            
            // remember compare val
            tempcompare = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
            
            // call the callback
            abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]();
            
            //==== step 2. Reschedule the timers which fired and need to be rescheduled
            
            // reschedule automatically if wasn't changed during callback
            if (abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]==tempcompare) {
               // keep previous value
               //poipoiabstimer_vars.radiotimer_overflow_previousVal = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW];
               
               // this is a periodic timer, reschedule automatically
               abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW] += abstimer_vars.radiotimer_period;
               
               // poipoipoi
               //poipoiabstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]  += abstimer_vars.radiotimer_period;
            }
                     
            // clear the interrupt flag
            abstimer_vars.bitmapInterruptsFired &= ~(1<<ABSTIMER_SRC_RADIOTIMER_OVERFLOW);
         
         } else if (abstimer_vars.bitmapInterruptsFired & (1<<ABSTIMER_SRC_RADIOTIMER_COMPARE)) {
            
            // update debug stats
            abstimer_dbg.num_radiotimer_compare++;
            
            // remember compare val
            tempcompare = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE];
            
            // call the callback
            abstimer_vars.callback[ABSTIMER_SRC_RADIOTIMER_COMPARE]();
            
            // reschedule automatically if wasn't changed during callback
            if (abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]==tempcompare) {
               
               // reschedule automatically after *overflow*
               abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_COMPARE]  = abstimer_vars.compareVal[ABSTIMER_SRC_RADIOTIMER_OVERFLOW]+ \
                                                                            abstimer_vars.radiotimer_compare_offset;
            }
            
            // clear the interrupt flag
            abstimer_vars.bitmapInterruptsFired &= ~(1<<ABSTIMER_SRC_RADIOTIMER_COMPARE);
         
         } else if (abstimer_vars.bitmapInterruptsFired & (1<<ABSTIMER_SRC_BSP_TIMER)) {
            
            // update debug stats
            abstimer_dbg.num_bsp_timer++;
            
            // call the callback
            abstimer_vars.callback[ABSTIMER_SRC_BSP_TIMER]();
            
            // clear the interrupt flag
            abstimer_vars.bitmapInterruptsFired &= ~(1<<ABSTIMER_SRC_BSP_TIMER);
            
            // no automatic rescheduling
         
         }
      }
   
      // make sure all interrupts have been handled
      if (abstimer_vars.bitmapInterruptsFired!=0) {
         while(1);
      }
   
      //===== step 4. schedule the next operation
      
      abstimer_reschedule();
   
      //===== step 5. make sure I'm not late for my next schedule
   
      // evaluate how much time has passed since theoretical currentTime,
      // (over estimate by ABSTIMER_GUARD_TICKS)
      timeSpent   = sctimer_getValue(); 
      timeSpent2  = (uint16_t)timeSpent-(uint16_t)abstimer_vars.currentTime;   
      timeSpent2 += ABSTIMER_GUARD_TICKS;
      
      // verify that, for each timer, that duration doesn't exceed how much time is left
      
      //find smallest next timer and mark it in the bitmap to be executed next. Can only be one marked at a time. 
      min=0xFFFF;
      for (i=0;i<ABSTIMER_SRC_MAX;i++) {
   
         // calculate distance to next timeout
         calc = (uint16_t)abstimer_vars.compareVal[i]-(uint16_t)abstimer_vars.currentTime; 
         
         if ((abstimer_vars.isArmed[i]==TRUE) && (timeSpent2>calc) && (calc<min)) {
            //this is the nearest in time
            min=calc;
            // this interrupt needs to be serviced now
            abstimer_vars.bitmapInterruptsFired = (1<<i);
            // update the next timeout value
            abstimer_vars.nextCurrentTime=abstimer_vars.compareVal[i];  
            // update debug statistics
            abstimer_dbg.num_late_schedule++;
         }
      }
      if (abstimer_vars.bitmapInterruptsFired!=0) {
         abstimer_vars.currentTime = abstimer_vars.nextCurrentTime;   
      }
   }
 
   debugpins_radio_clr();
   
   // kick the OS
   return 1;
}