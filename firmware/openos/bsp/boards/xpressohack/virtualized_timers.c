/**
\brief generic virtualization of the "timer" bsp module. We are going to use TIMER 2. We virtualize that timer in order to support N timers,

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */
#include "virtualized_timers.h"
#include "timer.h"
#include "bsp_timers.h"
#include "string.h"
#include "debugpins.h"


#define  NULL_TIMEOUT     0xFFFFFFFFu

Timer_t m_timers[NUM_TIMERS];
Bool running;
uint32_t    currentTimeout;     /* Current timeout in number of cycles.                     */

void virtualized_timer_compare_isr_0(uint8_t reg);
/**
 * init data structures and hardware timer. we are going to use hardware timer 2
 */
void virtualized_timers_init(){
	uint8_t i=0;
	running=FALSE;
	//init the data structure.

	while (i < NUM_TIMERS){
		m_timers[i].id=i;
		m_timers[i].period=0;
		m_timers[i].timer_remaining=0;
		m_timers[i].type=TIMER_ONESHOT; //whether is periodic or one shot
		m_timers[i].isrunning=FALSE; //is running?
		m_timers[i].fire=FALSE; //whether the function has to be called.
		i++;
	}

	timer_set_isr_compare_hook(TIMER_NUM2,virtualized_timer_compare_isr_0);//only one compare reg is used.

	timer_init(TIMER_NUM2);
	timer_enable(TIMER_NUM2);

}

/**
 * the timer works as follows:
 * + the currentTimeout points to the timer that is going to expire.
 * + if a new timer is inserted. we need to check that it is not earlier than the soonest.
 * + if it is earliest, just replace it.
 * + if not, insert it in the list.
 */

uint8_t virtualized_timers_start(uint16_t duration, timer_type_t type, timer_cbt callback){

	uint8_t i=0;
	uint8_t id=0;
	uint32_t cval=0; //current counter value

	//find first unused timer
	for (i=0; i<NUM_TIMERS && TRUE == m_timers[i].isrunning; i++) {}

	if (i<NUM_TIMERS){ //at least one is not used. let's use it.
		id=m_timers[i].id;
		cval=timer_get_current_value(TIMER_NUM2); //current time

		if(running==TRUE){ //at least one timer is running
			m_timers[i].timer_remaining=duration*TICS_PER_MS;
			if ( m_timers[i].timer_remaining<currentTimeout-cval){ //the new timer is earlier than current
				//set new timeout
				currentTimeout= m_timers[i].timer_remaining;
				//timer_reset(TIMER_NUM2);
				timer_set_compare(TIMER_NUM2,TIMER_COMPARE_REG0,m_timers[i].timer_remaining);

			}else{
				//not earlier.. do nothing, will be inserted in the list below
			}
		}else{//no timers running. this is the first one.

			m_timers[i].timer_remaining=duration* TICS_PER_MS;
			currentTimeout= m_timers[i].timer_remaining;
			timer_set_compare(TIMER_NUM2,TIMER_COMPARE_REG0,m_timers[i].timer_remaining);
			running=TRUE; //one timer is running at least.

		}
		//general sets in both cases:
		m_timers[i].isrunning=TRUE;
		m_timers[i].callback=callback;
		m_timers[i].fire=FALSE;//not fired yet!
		m_timers[i].type=type;
		m_timers[i].period=duration*TICS_PER_MS;
	}else{
		return TOO_MANY_TIMERS_ERROR;
	}

	return id;
}


/*
 * cancelling it is a matter of setting the timer to not running.
 * if it was the nexttimeout, the isr will find another and also
 * will substract the elapsed time to other running timers.
 */
void virtualized_timers_stop(uint8_t id){
	uint8_t i=0;
	Bool end=FALSE;
	while ( i<NUM_TIMERS && !end ){
		if (id==m_timers[i].id){
			end=TRUE;
			m_timers[i].isrunning=FALSE;
		}
	i++;
	}
}






/**
  \brief Interrupt handler for TIMER_NUM2, compare
  when the interrupt comes, we need to:
  +find all the timers that have expired and mark them as pending to be fired.
  +update all the remaining time of the pending timers
  +select the next timer.

 */
void virtualized_timer_compare_isr_0(uint8_t reg) {

	debugpins_task_toggle();
	uint8_t i=0;
	uint32_t next_timeout;
	uint32_t counterval;

   // timer_disable(TIMER_NUM2);
	counterval=timer_get_current_value(TIMER_NUM2);
	timer_reset(TIMER_NUM2);
	timer_enable(TIMER_NUM2);
	next_timeout=NULL_TIMEOUT;


	for(i=0; i<NUM_TIMERS; i++) {//look for the expired timers
		if(TRUE == m_timers[i].isrunning) {
			if(currentTimeout >= m_timers[i].timer_remaining) {        /* If timeout ended               */
				m_timers[i].fire=TRUE;//set as fired
			}
			else {//this timer is not expired. just update its counter.
				//the time of ISR is 57.25uS so I should compensate this time.
				if ((currentTimeout+(currentTimeout-counterval))+1<m_timers[i].timer_remaining){
					m_timers[i].timer_remaining -= (currentTimeout+(currentTimeout-counterval));//substract the amount elapsed to all timers (+correct time of isr)
				}else{
					m_timers[i].timer_remaining=0;//set to zero as this timer also expired during the isr.
					m_timers[i].fire=TRUE;//set as fired
				}
			}
		}
	}//end for

	//call all the callbacks...
	for(i=0; i<NUM_TIMERS; i++) {
		if (TRUE==m_timers[i].fire){//if needs to be fired
			//TODO.. this is not correct. Have a task pending from a sempahore and singal the semaphore.
			//the task to be executed goes through all timers and executes the functions.
			m_timers[i].callback();
			m_timers[i].fire=FALSE;//set already fired!
			if (m_timers[i].type==TIMER_ONESHOT) {
				m_timers[i].isrunning = FALSE;//close the timer.
			}else{//reset the period.
				m_timers[i].timer_remaining=m_timers[i].period;//will be selected next if applies below.
			}
		}

	}
	/* Sleep not done, calculate new timeout.              */
	for(i=0; i<NUM_TIMERS; i++) {
		if(next_timeout > m_timers[i].timer_remaining && m_timers[i].isrunning==TRUE) {
			next_timeout = m_timers[i].timer_remaining; //get the minimum to find the next timer
		}
	}
	//schedule the next timer as in the callback a new one can be added.
	if (NULL_TIMEOUT != next_timeout) {//there is another timer pending
		currentTimeout = next_timeout;
		timer_set_compare(TIMER_NUM2,TIMER_COMPARE_REG0,currentTimeout);//schedule it
		running = TRUE;//we keep running
	}
	else {
		running = FALSE;//no more timers.. then No running.
	}
	//timer_enable(TIMER_NUM2);
	debugpins_task_toggle();
}

