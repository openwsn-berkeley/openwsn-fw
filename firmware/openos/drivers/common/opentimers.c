/**
\brief Definition of the "opentimers" driver.

This driver uses a single hardware timer, which it virtualizes to support
at most MAX_NUM_TIMERS timers.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/

#include "openwsn.h"
#include "board_info.h"
#include "opentimers.h"
#include "bsp_timers.h"
#include "string.h"

//=========================== define ==========================================

#define  NULL_TIMEOUT     0xFFFFFFFFu

//=========================== variables =======================================

typedef struct {
   opentimers_t         timersBuf[MAX_NUM_TIMERS];
   bool                 running;
   PORT_TIMER_WIDTH     currentTimeout; // current timeout, in ticks
} opentimers_vars_t;

opentimers_vars_t opentimers_vars;

//=========================== prototypes ======================================

void opentimers_timer_callback();

//=========================== public ==========================================

/**
\brief Initialize this module.

Initializes data structures and hardware timer.
*/
void opentimers_init(){
   uint8_t i;
   
   opentimers_vars.running=FALSE;
   
   for (i=0;i<MAX_NUM_TIMERS;i++) {
      opentimers_vars.timersBuf[i].id                 = i;
      opentimers_vars.timersBuf[i].period             = 0;
      opentimers_vars.timersBuf[i].timer_remaining    = 0;
      opentimers_vars.timersBuf[i].type               = TIMER_ONESHOT;
      opentimers_vars.timersBuf[i].isrunning          = FALSE;
      opentimers_vars.timersBuf[i].callback           = NULL;
      opentimers_vars.timersBuf[i].hasFired           = FALSE;
   }

   bsp_timers_set_callback(opentimers_timer_callback);
}

/**
\brief Start a timer.

The timer works as follows:
- the currentTimeout points to the timer that is going to expire.
- if a new timer is inserted. we need to check that it is not earlier than the soonest.
- if it is earliest, just replace it.
- if not, insert it in the list.

\param duration Number milli-seconds after which the timer will fire.
\param type     The type of timer, indicating whether it's a one-shot or a period timer.
\param callback The function to call when the timer fires.

\returns The id of the timer (which serves as a handler to stop it) if the
         timer could be started.
\returns TOO_MANY_TIMERS_ERROR if the timer could NOT be started.
*/
opentimer_id_t opentimers_start(uint16_t duration, timer_type_t type, opentimers_cbt callback) {

   uint8_t  i;
   uint8_t  id;
   uint32_t cval; //<current counter value
   
   i          = 0;
   id         = 0;
   cval       = 0;

   // find an unused timer
   for (i=0; i<MAX_NUM_TIMERS && opentimers_vars.timersBuf[i].isrunning==TRUE; i++);

   if (i<MAX_NUM_TIMERS) {
      // we found an unused timer
      
      id      = opentimers_vars.timersBuf[i].id;
      cval    = bsp_timers_get_current_value(); //current time

      if (opentimers_vars.running==TRUE) {
         // at least one timer is running
         
         opentimers_vars.timersBuf[i].timer_remaining = duration*PORT_TICS_PER_MS;
         
         if (opentimers_vars.timersBuf[i].timer_remaining<opentimers_vars.currentTimeout-cval) {
            //the new timer is earlier than current
            
            //set new timeout
            opentimers_vars.currentTimeout   = opentimers_vars.timersBuf[i].timer_remaining;
            bsp_timers_set_compare(opentimers_vars.timersBuf[i].timer_remaining);
         } else {
            //not earlier. do nothing, will be inserted in the list below.
         }
      } else {
         // no timers arerunning. this is the first one.

         opentimers_vars.timersBuf[i].timer_remaining = duration*PORT_TICS_PER_MS;
         opentimers_vars.currentTimeout              = opentimers_vars.timersBuf[i].timer_remaining;
         bsp_timers_set_compare(opentimers_vars.timersBuf[i].timer_remaining);
         opentimers_vars.running                     = TRUE; // at least one timer is running

      }
      
      //general sets in both cases:
      opentimers_vars.timersBuf[i].isrunning  = TRUE;
      opentimers_vars.timersBuf[i].callback   = callback;
      opentimers_vars.timersBuf[i].hasFired   = FALSE;
      opentimers_vars.timersBuf[i].type       = type;
      opentimers_vars.timersBuf[i].period     = duration*PORT_TICS_PER_MS;
   
   } else {
      return TOO_MANY_TIMERS_ERROR;
   }
   
   return id;
}

/**

*/
void  opentimers_setPeriod(opentimer_id_t id,
                           uint16_t       newPeriod) {
  opentimer_id_t i;
   
   i = 0;
   while (i<MAX_NUM_TIMERS) {
      if (id==opentimers_vars.timersBuf[i].id){
         opentimers_vars.timersBuf[i].period=newPeriod;
         break;
      }
   }
}

/**
\brief Stop a running timer.

Sets the timer to "not running". If it was the timer to timeout next, the isr
will find another and also will substract the elapsed time to other running timers.

\param The ID of the timer, as returned by opentimers_start().
*/
void opentimers_stop(opentimer_id_t id) {
   opentimer_id_t i;
   
   i = 0;
   while (i<MAX_NUM_TIMERS) {
      if (id==opentimers_vars.timersBuf[i].id){
         opentimers_vars.timersBuf[i].isrunning=FALSE;
         break;
      }
   }
}

//=========================== private =========================================

/**
\brief callback function when timer compare happens.

Executed in interrupt mode

When this callback is fired, weneed to:
- find all the timers that have expired and mark them as pending to be fired.
- update all the remaining time of the pending timers
- select the next timer.
*/
void opentimers_timer_callback() {
   
   uint8_t i=0;
   uint32_t next_timeout;
   uint32_t counterval;
   
   counterval=bsp_timers_get_current_value();
   //poipoitimer_reset(TIMER_NUM2);
   //poipoitimer_enable(TIMER_NUM2);
   next_timeout=NULL_TIMEOUT;

   for(i=0; i<MAX_NUM_TIMERS; i++) {//look for the expired timers
      if(TRUE == opentimers_vars.timersBuf[i].isrunning) {
         if(opentimers_vars.currentTimeout >= opentimers_vars.timersBuf[i].timer_remaining) {        /* If timeout ended               */
            opentimers_vars.timersBuf[i].hasFired=TRUE;//set as fired
         }
         else {//this timer is not expired. just update its counter.
            //the time of ISR is 57.25uS so I should compensate this time.
            if ((opentimers_vars.currentTimeout+(opentimers_vars.currentTimeout-counterval))+1<opentimers_vars.timersBuf[i].timer_remaining){
               opentimers_vars.timersBuf[i].timer_remaining -= (opentimers_vars.currentTimeout+(opentimers_vars.currentTimeout-counterval));//substract the amount elapsed to all timers (+correct time of isr)
            }else{
               opentimers_vars.timersBuf[i].timer_remaining=0;//set to zero as this timer also expired during the isr.
               opentimers_vars.timersBuf[i].hasFired=TRUE;//set as fired
            }
         }
      }
   }//end for

   //call all the callbacks...
   for(i=0; i<MAX_NUM_TIMERS; i++) {
      if (TRUE==opentimers_vars.timersBuf[i].hasFired){//if needs to be fired
         //TODO.. this is not correct. Have a task pending from a sempahore and singal the semaphore.
         //the task to be executed goes through all timers and executes the functions.
         opentimers_vars.timersBuf[i].callback();
         opentimers_vars.timersBuf[i].hasFired=FALSE;//set already fired!
         if (opentimers_vars.timersBuf[i].type==TIMER_ONESHOT) {
            opentimers_vars.timersBuf[i].isrunning = FALSE;//close the timer.
         }else{//reset the period.
            opentimers_vars.timersBuf[i].timer_remaining=opentimers_vars.timersBuf[i].period;//will be selected next if applies below.
         }
      }

   }
   /* Sleep not done, calculate new timeout.              */
   for(i=0; i<MAX_NUM_TIMERS; i++) {
      if(next_timeout > opentimers_vars.timersBuf[i].timer_remaining && opentimers_vars.timersBuf[i].isrunning==TRUE) {
         next_timeout = opentimers_vars.timersBuf[i].timer_remaining; //get the minimum to find the next timer
      }
   }
   //schedule the next timer as in the callback a new one can be added.
   if (NULL_TIMEOUT != next_timeout) {//there is another timer pending
      opentimers_vars.currentTimeout = next_timeout;
      bsp_timers_set_compare(opentimers_vars.currentTimeout);//schedule it
      opentimers_vars.running = TRUE;//we keep running
   }
   else {
      opentimers_vars.running = FALSE;//no more timers.. then No running.
   }
   //timer_enable(TIMER_NUM2);
}

