/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:27.201776.
*/
/**
\brief Definition of the "opentimers" driver.

This driver uses a single hardware timer, which it virtualizes to support
at most MAX_NUM_TIMERS timers.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */

#include "openwsn_obj.h"
#include "opentimers_obj.h"
#include "bsp_timer_obj.h"
#include "leds_obj.h"

//=========================== define ==========================================

//=========================== variables =======================================

// declaration of global variable _opentimers_vars_ removed during objectification.
//uint32_t counter; //counts the elapsed time.

//=========================== prototypes ======================================

void opentimers_timer_callback(OpenMote* self);

//=========================== public ==========================================

/**
\brief Initialize this module.

Initializes data structures and hardware timer.
 */
void opentimers_init(OpenMote* self){
   uint8_t i;

   // initialize local variables
   (self->opentimers_vars).running=FALSE;
   for (i=0;i<MAX_NUM_TIMERS;i++) {
      (self->opentimers_vars).timersBuf[i].period_ticks       = 0;
      (self->opentimers_vars).timersBuf[i].ticks_remaining    = 0;
      (self->opentimers_vars).timersBuf[i].type               = TIMER_ONESHOT;
      (self->opentimers_vars).timersBuf[i].isrunning          = FALSE;
      (self->opentimers_vars).timersBuf[i].callback           = NULL;
      (self->opentimers_vars).timersBuf[i].hasExpired         = FALSE;
   }

   // set callback for bsp_timers module
 bsp_timer_set_callback(self, opentimers_timer_callback);
}

/**
\brief Start a timer.

The timer works as follows:
- currentTimeout is the number of ticks before the next timer expires.
- if a new timer is inserted, we check that it is not earlier than the soonest
- if it is earliest, replace it
- if not, insert it in the list

\param duration Number milli-seconds after which the timer will fire.
\param type     Type of timer:
   - #TIMER_PERIODIC for a periodic timer.
   - #TIMER_ONESHOT for a on-shot timer.
\param timetype Units of the <tt>duration</tt>:
   - #TIME_MS when <tt>duration</tt> is in ms.
   - #TIME_TICS when <tt>duration</tt> is in clock ticks.
\param callback The function to call when the timer fires.

\returns The id of the timer (which serves as a handler to stop it) if the
         timer could be started.
\returns TOO_MANY_TIMERS_ERROR if the timer could NOT be started.
 */
opentimer_id_t opentimers_start(OpenMote* self, uint32_t duration, timer_type_t type, time_type_t timetype, opentimers_cbt callback) {

   uint8_t  id;

   // find an unused timer
   for (id=0; id<MAX_NUM_TIMERS && (self->opentimers_vars).timersBuf[id].isrunning==TRUE; id++);

   if (id<MAX_NUM_TIMERS) {
      // we found an unused timer

      // register the timer
      if (timetype==TIME_MS) {
         (self->opentimers_vars).timersBuf[id].period_ticks      = duration*PORT_TICS_PER_MS;
         (self->opentimers_vars).timersBuf[id].wraps_remaining   = (duration*PORT_TICS_PER_MS/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
      } else if (timetype==TIME_TICS) {
         (self->opentimers_vars).timersBuf[id].period_ticks      = duration;
         (self->opentimers_vars).timersBuf[id].wraps_remaining   = (duration/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t  
      } else {
         while (1); //error
      }
      //if the number of ticks falls below a 16bit value, use it, otherwise set to max 16bit value
      if((self->opentimers_vars).timersBuf[id].wraps_remaining==0){
         if (timetype==TIME_MS){ 
            (self->opentimers_vars).timersBuf[id].ticks_remaining   = duration*PORT_TICS_PER_MS;
         } else if (timetype==TIME_TICS) {
            (self->opentimers_vars).timersBuf[id].ticks_remaining   = duration;
         } else {
            // this should never happpen!
   
            // we can not print from within the drivers. Instead:
            // blink the error LED
 leds_error_blink(self);
            // reset the board
 board_reset(self);
         }
      }else{
         (self->opentimers_vars).timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;
      }                                                   
      (self->opentimers_vars).timersBuf[id].type              = type;
      (self->opentimers_vars).timersBuf[id].isrunning         = TRUE;
      (self->opentimers_vars).timersBuf[id].callback          = callback;
      (self->opentimers_vars).timersBuf[id].hasExpired        = FALSE;

      // re-schedule the running timer, if needed
      if (
            ((self->opentimers_vars).running==FALSE)
            ||
            ((self->opentimers_vars).timersBuf[id].ticks_remaining < (self->opentimers_vars).currentTimeout)
      ) {  
         (self->opentimers_vars).currentTimeout            = (self->opentimers_vars).timersBuf[id].ticks_remaining;
         if ((self->opentimers_vars).running==FALSE) {
 bsp_timer_reset(self);
         }
 bsp_timer_scheduleIn(self, (self->opentimers_vars).timersBuf[id].ticks_remaining);
      }

      (self->opentimers_vars).running                         = TRUE;

   } else {
      return TOO_MANY_TIMERS_ERROR;
   }

   return id;
}

/**
\brief Replace the period of a running timer.
 */
void opentimers_setPeriod(OpenMote* self, opentimer_id_t id,time_type_t timetype,uint32_t newDuration) {
   if        (timetype==TIME_MS) {
      (self->opentimers_vars).timersBuf[id].period_ticks      = newDuration*PORT_TICS_PER_MS;
      (self->opentimers_vars).timersBuf[id].wraps_remaining   = (newDuration*PORT_TICS_PER_MS/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
   } else if (timetype==TIME_TICS) {
      (self->opentimers_vars).timersBuf[id].period_ticks      = newDuration;
      (self->opentimers_vars).timersBuf[id].wraps_remaining   = (newDuration/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
   } else {
      // this should never happpen!
      
      // we can not print from within the drivers. Instead:
      // blink the error LED
 leds_error_blink(self);
      // reset the board
 board_reset(self);
   }
   if((self->opentimers_vars).timersBuf[id].wraps_remaining==0) {
      if        (timetype==TIME_MS){
         (self->opentimers_vars).timersBuf[id].ticks_remaining   = newDuration*PORT_TICS_PER_MS;
      } else if (timetype==TIME_TICS){
         (self->opentimers_vars).timersBuf[id].ticks_remaining   = newDuration;
      }
   } else {
      (self->opentimers_vars).timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;
   }
}

/**
\brief Stop a running timer.

Sets the timer to "not running". the system recovers even if this was the next
timer to expire.
 */
void opentimers_stop(OpenMote* self, opentimer_id_t id) {
   (self->opentimers_vars).timersBuf[id].isrunning=FALSE;
}

/**
\brief Restart a stop timer.

Sets the timer to " running".
 */
void opentimers_restart(OpenMote* self, opentimer_id_t id) {
   (self->opentimers_vars).timersBuf[id].isrunning=TRUE;
}


//=========================== private =========================================

/**
\brief Function called when the hardware timer expires.

Executed in interrupt mode.

This function maps the expiration event to possibly multiple timers, calls the
corresponding callback(s), and restarts the hardware timer with the next timer
to expire.
 */
void opentimers_timer_callback(OpenMote* self) {
   
   opentimer_id_t   id;
   PORT_TIMER_WIDTH min_timeout;
   bool             found;
    
   // step 1. Identify expired timers
   for(id=0; id<MAX_NUM_TIMERS; id++) {
      if ((self->opentimers_vars).timersBuf[id].isrunning==TRUE) {

         if((self->opentimers_vars).currentTimeout >= (self->opentimers_vars).timersBuf[id].ticks_remaining) {
            // this timer has expired
            //check to see if we have completed the whole timer, or we're just wrapping around the max 16bit value
            if((self->opentimers_vars).timersBuf[id].wraps_remaining == 0){
               // declare as so
               (self->opentimers_vars).timersBuf[id].hasExpired  = TRUE;
            }else{
               (self->opentimers_vars).timersBuf[id].wraps_remaining--;
               if((self->opentimers_vars).timersBuf[id].wraps_remaining == 0){
                  //if we have fully wrapped around, then set the remainring ticks to the modulus of the total ticks and the max clock value
                  (self->opentimers_vars).timersBuf[id].ticks_remaining = ((self->opentimers_vars).timersBuf[id].period_ticks) % MAX_TICKS_IN_SINGLE_CLOCK;
               }else{
                  (self->opentimers_vars).timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;
               }
            }
         } else {
            // this timer is not expired

            // update its counter
            (self->opentimers_vars).timersBuf[id].ticks_remaining -= (self->opentimers_vars).currentTimeout;
         }   
      }
   }

   // step 2. call callbacks of expired timers
   for(id=0; id<MAX_NUM_TIMERS; id++) {
      if ((self->opentimers_vars).timersBuf[id].hasExpired==TRUE){

         // call the callback
         (self->opentimers_vars).timersBuf[id].callback(self);
         (self->opentimers_vars).timersBuf[id].hasExpired     = FALSE;

         // reload the timer, if applicable
         if ((self->opentimers_vars).timersBuf[id].type==TIMER_PERIODIC) {
            (self->opentimers_vars).timersBuf[id].wraps_remaining   = ((self->opentimers_vars).timersBuf[id].period_ticks/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
            //if the number of ticks falls below a 16bit value, use it, otherwise set to max 16bit value
            if((self->opentimers_vars).timersBuf[id].wraps_remaining==0)                                                
               (self->opentimers_vars).timersBuf[id].ticks_remaining   = (self->opentimers_vars).timersBuf[id].period_ticks;
            else
               (self->opentimers_vars).timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;

         } else {
            (self->opentimers_vars).timersBuf[id].isrunning   = FALSE;
         }
      }

   }

   // step 3. find the minimum remaining timeout among running timers
   found = FALSE;
   for(id=0;id<MAX_NUM_TIMERS;id++) {
      if (
            (self->opentimers_vars).timersBuf[id].isrunning==TRUE &&
            (
                  found==FALSE
                  ||
                  (self->opentimers_vars).timersBuf[id].ticks_remaining < min_timeout
            )
      ) {
         min_timeout    = (self->opentimers_vars).timersBuf[id].ticks_remaining;
         found          = TRUE;
      }
   }

   // step 4. schedule next timeout
   if (found==TRUE) {
      // at least one timer pending
      (self->opentimers_vars).currentTimeout = min_timeout;
 bsp_timer_scheduleIn(self, (self->opentimers_vars).currentTimeout);
   } else {
      // no more timers pending
      (self->opentimers_vars).running = FALSE;
   }
}

void opentimers_sleepTimeCompesation(OpenMote* self, uint16_t sleepTime)
{
   opentimer_id_t   id;
   PORT_TIMER_WIDTH min_timeout;
   bool             found;
   
   //step 1. reCount the ticks_remain after waking up from sleep
   for(id=0; id<MAX_NUM_TIMERS; id++)
   {
     if ((self->opentimers_vars).timersBuf[id].isrunning==TRUE) 
     {
       if((self->opentimers_vars).timersBuf[id].ticks_remaining > sleepTime)
       {
         (self->opentimers_vars).timersBuf[id].ticks_remaining -= sleepTime;
       }
       else
       {
         if((self->opentimers_vars).timersBuf[id].wraps_remaining > 0)
         {
           (self->opentimers_vars).timersBuf[id].wraps_remaining--;
           (self->opentimers_vars).timersBuf[id].ticks_remaining += (MAX_TICKS_IN_SINGLE_CLOCK - sleepTime);
         }
         else
         {
           (self->opentimers_vars).timersBuf[id].hasExpired  = TRUE;
         }
       }
     }
   }
   
   // step 2. call callbacks of expired timers
   for(id=0; id<MAX_NUM_TIMERS; id++) {
      if ((self->opentimers_vars).timersBuf[id].hasExpired==TRUE){

         // call the callback
         (self->opentimers_vars).timersBuf[id].callback(self);
         (self->opentimers_vars).timersBuf[id].hasExpired     = FALSE;

         // reload the timer, if applicable
         if ((self->opentimers_vars).timersBuf[id].type==TIMER_PERIODIC) {
            (self->opentimers_vars).timersBuf[id].wraps_remaining   = ((self->opentimers_vars).timersBuf[id].period_ticks/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
            //if the number of ticks falls below a 16bit value, use it, otherwise set to max 16bit value
            if((self->opentimers_vars).timersBuf[id].wraps_remaining==0)                                                
               (self->opentimers_vars).timersBuf[id].ticks_remaining   = (self->opentimers_vars).timersBuf[id].period_ticks;
            else
               (self->opentimers_vars).timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;

         } else {
            (self->opentimers_vars).timersBuf[id].isrunning   = FALSE;
         }
      }

   }
   
   // step 3. find the minimum remaining timeout among running timers
   found = FALSE;
   for(id=0;id<MAX_NUM_TIMERS;id++) {
      if (
            (self->opentimers_vars).timersBuf[id].isrunning==TRUE &&
            (
                  found==FALSE
                  ||
                  (self->opentimers_vars).timersBuf[id].ticks_remaining < min_timeout
            )
      ) {
         min_timeout    = (self->opentimers_vars).timersBuf[id].ticks_remaining;
         found          = TRUE;
      }
   }

   // step 4. schedule next timeout
   if (found==TRUE) {
      // at least one timer pending
      (self->opentimers_vars).currentTimeout = min_timeout;
 bsp_timer_scheduleIn(self, (self->opentimers_vars).currentTimeout);
   } else {
      // no more timers pending
      (self->opentimers_vars).running = FALSE;
   }
}
