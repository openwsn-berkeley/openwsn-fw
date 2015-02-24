/**
\brief Definition of the "opentimers" driver.

This driver uses a single hardware timer, which it virtualizes to support
at most MAX_NUM_TIMERS timers.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */

#include "opendefs.h"
#include "opentimers.h"
#include "bsp_timer.h"
#include "leds.h"

//=========================== define ==========================================

//=========================== variables =======================================

opentimers_vars_t opentimers_vars;
//uint32_t counter; //counts the elapsed time.

//=========================== prototypes ======================================

void opentimers_timer_callback(void);

//=========================== public ==========================================

/**
\brief Initialize this module.

Initializes data structures and hardware timer.
 */
void opentimers_init(){
   uint8_t i;

   // initialize local variables
   opentimers_vars.running=FALSE;
   for (i=0;i<MAX_NUM_TIMERS;i++) {
      opentimers_vars.timersBuf[i].period_ticks       = 0;
      opentimers_vars.timersBuf[i].ticks_remaining    = 0;
      opentimers_vars.timersBuf[i].type               = TIMER_ONESHOT;
      opentimers_vars.timersBuf[i].isrunning          = FALSE;
      opentimers_vars.timersBuf[i].callback           = NULL;
      opentimers_vars.timersBuf[i].hasExpired         = FALSE;
   }

   // set callback for bsp_timers module
   bsp_timer_set_callback(opentimers_timer_callback);
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
opentimer_id_t opentimers_start(uint32_t duration, timer_type_t type, time_type_t timetype, opentimers_cbt callback) {

   uint8_t  id;

   // find an unused timer
   for (id=0; id<MAX_NUM_TIMERS && opentimers_vars.timersBuf[id].isrunning==TRUE; id++);

   if (id<MAX_NUM_TIMERS) {
      // we found an unused timer

      // register the timer
      if (timetype==TIME_MS) {
         opentimers_vars.timersBuf[id].period_ticks      = duration*PORT_TICS_PER_MS;
         opentimers_vars.timersBuf[id].wraps_remaining   = (duration*PORT_TICS_PER_MS/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
      } else if (timetype==TIME_TICS) {
         opentimers_vars.timersBuf[id].period_ticks      = duration;
         opentimers_vars.timersBuf[id].wraps_remaining   = (duration/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t  
      } else {
         while (1); //error
      }
      //if the number of ticks falls below a 16bit value, use it, otherwise set to max 16bit value
      if(opentimers_vars.timersBuf[id].wraps_remaining==0){
         if (timetype==TIME_MS){ 
            opentimers_vars.timersBuf[id].ticks_remaining   = duration*PORT_TICS_PER_MS;
         } else if (timetype==TIME_TICS) {
            opentimers_vars.timersBuf[id].ticks_remaining   = duration;
         } else {
            // this should never happpen!
   
            // we can not print from within the drivers. Instead:
            // blink the error LED
            leds_error_blink();
            // reset the board
            board_reset();
         }
      }else{
         opentimers_vars.timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;
      }                                                   
      opentimers_vars.timersBuf[id].type              = type;
      opentimers_vars.timersBuf[id].isrunning         = TRUE;
      opentimers_vars.timersBuf[id].callback          = callback;
      opentimers_vars.timersBuf[id].hasExpired        = FALSE;

      // re-schedule the running timer, if needed
      if (
            (opentimers_vars.running==FALSE)
            ||
            (opentimers_vars.timersBuf[id].ticks_remaining < opentimers_vars.currentTimeout)
      ) {  
         opentimers_vars.currentTimeout            = opentimers_vars.timersBuf[id].ticks_remaining;
         if (opentimers_vars.running==FALSE) {
            bsp_timer_reset();
         }
         bsp_timer_scheduleIn(opentimers_vars.timersBuf[id].ticks_remaining);
      }

      opentimers_vars.running                         = TRUE;

   } else {
      return TOO_MANY_TIMERS_ERROR;
   }

   return id;
}

/**
\brief Replace the period of a running timer.
 */
void  opentimers_setPeriod(opentimer_id_t id,time_type_t timetype,uint32_t newDuration) {
   if        (timetype==TIME_MS) {
      opentimers_vars.timersBuf[id].period_ticks      = newDuration*PORT_TICS_PER_MS;
      opentimers_vars.timersBuf[id].wraps_remaining   = (newDuration*PORT_TICS_PER_MS/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
   } else if (timetype==TIME_TICS) {
      opentimers_vars.timersBuf[id].period_ticks      = newDuration;
      opentimers_vars.timersBuf[id].wraps_remaining   = (newDuration/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
   } else {
      // this should never happpen!
      
      // we can not print from within the drivers. Instead:
      // blink the error LED
      leds_error_blink();
      // reset the board
      board_reset();
   }
   if(opentimers_vars.timersBuf[id].wraps_remaining==0) {
      if        (timetype==TIME_MS){
         opentimers_vars.timersBuf[id].ticks_remaining   = newDuration*PORT_TICS_PER_MS;
      } else if (timetype==TIME_TICS){
         opentimers_vars.timersBuf[id].ticks_remaining   = newDuration;
      }
   } else {
      opentimers_vars.timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;
   }
}

/**
\brief Stop a running timer.

Sets the timer to "not running". the system recovers even if this was the next
timer to expire.
 */
void opentimers_stop(opentimer_id_t id) {
   opentimers_vars.timersBuf[id].isrunning=FALSE;
}

/**
\brief Restart a stop timer.

Sets the timer to " running".
 */
void opentimers_restart(opentimer_id_t id) {
   opentimers_vars.timersBuf[id].isrunning=TRUE;
}


//=========================== private =========================================

/**
\brief Function called when the hardware timer expires.

Executed in interrupt mode.

This function maps the expiration event to possibly multiple timers, calls the
corresponding callback(s), and restarts the hardware timer with the next timer
to expire.
 */
void opentimers_timer_callback() {
   
   opentimer_id_t   id;
   PORT_TIMER_WIDTH min_timeout;
   bool             found;
    
   // step 1. Identify expired timers
   for(id=0; id<MAX_NUM_TIMERS; id++) {
      if (opentimers_vars.timersBuf[id].isrunning==TRUE) {

         if(opentimers_vars.currentTimeout >= opentimers_vars.timersBuf[id].ticks_remaining) {
            // this timer has expired
            //check to see if we have completed the whole timer, or we're just wrapping around the max 16bit value
            if(opentimers_vars.timersBuf[id].wraps_remaining == 0){
               // declare as so
               opentimers_vars.timersBuf[id].hasExpired  = TRUE;
            }else{
               opentimers_vars.timersBuf[id].wraps_remaining--;
               if(opentimers_vars.timersBuf[id].wraps_remaining == 0){
                  //if we have fully wrapped around, then set the remainring ticks to the modulus of the total ticks and the max clock value
                  opentimers_vars.timersBuf[id].ticks_remaining = (opentimers_vars.timersBuf[id].period_ticks) % MAX_TICKS_IN_SINGLE_CLOCK;
               }else{
                  opentimers_vars.timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;
               }
            }
         } else {
            // this timer is not expired

            // update its counter
            opentimers_vars.timersBuf[id].ticks_remaining -= opentimers_vars.currentTimeout;
         }   
      }
   }

   // step 2. call callbacks of expired timers
   for(id=0; id<MAX_NUM_TIMERS; id++) {
      if (opentimers_vars.timersBuf[id].hasExpired==TRUE){

         // call the callback
         opentimers_vars.timersBuf[id].callback(id);
         opentimers_vars.timersBuf[id].hasExpired     = FALSE;

         // reload the timer, if applicable
         if (opentimers_vars.timersBuf[id].type==TIMER_PERIODIC) {
            opentimers_vars.timersBuf[id].wraps_remaining   = (opentimers_vars.timersBuf[id].period_ticks/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
            //if the number of ticks falls below a 16bit value, use it, otherwise set to max 16bit value
            if(opentimers_vars.timersBuf[id].wraps_remaining==0)                                                
               opentimers_vars.timersBuf[id].ticks_remaining   = opentimers_vars.timersBuf[id].period_ticks;
            else
               opentimers_vars.timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;

         } else {
            opentimers_vars.timersBuf[id].isrunning   = FALSE;
         }
      }

   }

   // step 3. find the minimum remaining timeout among running timers
   found = FALSE;
   for(id=0;id<MAX_NUM_TIMERS;id++) {
      if (
            opentimers_vars.timersBuf[id].isrunning==TRUE &&
            (
                  found==FALSE
                  ||
                  opentimers_vars.timersBuf[id].ticks_remaining < min_timeout
            )
      ) {
         min_timeout    = opentimers_vars.timersBuf[id].ticks_remaining;
         found          = TRUE;
      }
   }

   // step 4. schedule next timeout
   if (found==TRUE) {
      // at least one timer pending
      opentimers_vars.currentTimeout = min_timeout;
      bsp_timer_scheduleIn(opentimers_vars.currentTimeout);
   } else {
      // no more timers pending
      opentimers_vars.running = FALSE;
   }
}

void opentimers_sleepTimeCompesation(uint16_t sleepTime)
{
   opentimer_id_t   id;
   PORT_TIMER_WIDTH min_timeout;
   bool             found;
   
   //step 1. reCount the ticks_remain after waking up from sleep
   for(id=0; id<MAX_NUM_TIMERS; id++)
   {
     if (opentimers_vars.timersBuf[id].isrunning==TRUE) 
     {
       if(opentimers_vars.timersBuf[id].ticks_remaining > sleepTime)
       {
         opentimers_vars.timersBuf[id].ticks_remaining -= sleepTime;
       }
       else
       {
         if(opentimers_vars.timersBuf[id].wraps_remaining > 0)
         {
           opentimers_vars.timersBuf[id].wraps_remaining--;
           opentimers_vars.timersBuf[id].ticks_remaining += (MAX_TICKS_IN_SINGLE_CLOCK - sleepTime);
         }
         else
         {
           opentimers_vars.timersBuf[id].hasExpired  = TRUE;
         }
       }
     }
   }
   
   // step 2. call callbacks of expired timers
   for(id=0; id<MAX_NUM_TIMERS; id++) {
      if (opentimers_vars.timersBuf[id].hasExpired==TRUE){

         // call the callback
         opentimers_vars.timersBuf[id].callback(id);
         opentimers_vars.timersBuf[id].hasExpired     = FALSE;

         // reload the timer, if applicable
         if (opentimers_vars.timersBuf[id].type==TIMER_PERIODIC) {
            opentimers_vars.timersBuf[id].wraps_remaining   = (opentimers_vars.timersBuf[id].period_ticks/MAX_TICKS_IN_SINGLE_CLOCK);//65535=maxValue of uint16_t
            //if the number of ticks falls below a 16bit value, use it, otherwise set to max 16bit value
            if(opentimers_vars.timersBuf[id].wraps_remaining==0)                                                
               opentimers_vars.timersBuf[id].ticks_remaining   = opentimers_vars.timersBuf[id].period_ticks;
            else
               opentimers_vars.timersBuf[id].ticks_remaining = MAX_TICKS_IN_SINGLE_CLOCK;

         } else {
            opentimers_vars.timersBuf[id].isrunning   = FALSE;
         }
      }

   }
   
   // step 3. find the minimum remaining timeout among running timers
   found = FALSE;
   for(id=0;id<MAX_NUM_TIMERS;id++) {
      if (
            opentimers_vars.timersBuf[id].isrunning==TRUE &&
            (
                  found==FALSE
                  ||
                  opentimers_vars.timersBuf[id].ticks_remaining < min_timeout
            )
      ) {
         min_timeout    = opentimers_vars.timersBuf[id].ticks_remaining;
         found          = TRUE;
      }
   }

   // step 4. schedule next timeout
   if (found==TRUE) {
      // at least one timer pending
      opentimers_vars.currentTimeout = min_timeout;
      bsp_timer_scheduleIn(opentimers_vars.currentTimeout);
   } else {
      // no more timers pending
      opentimers_vars.running = FALSE;
   }
}
