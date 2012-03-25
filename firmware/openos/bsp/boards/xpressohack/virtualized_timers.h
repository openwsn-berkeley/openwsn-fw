/**
\brief Cross-platform declaration "timer" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/


#ifndef VIRTUALIZED_TIMERS_H_
#define VIRTUALIZED_TIMERS_H_

#include "lpc_types.h"
#include "bsp_timer.h"


#define NUM_TIMERS 10 //10 is enough!
#define TOO_MANY_TIMERS_ERROR 255;


typedef struct
  {
	uint8_t id; //to identify the timer
    uint32_t period;//timeout
    uint32_t timer_remaining; //time remaining to signal isr
    timer_type_t type; //whether is periodic or one shot
    Bool isrunning; //is running?
    timer_cbt    callback; //isr function to call
    Bool fire; //whether the function has to be called.
  } Timer_t;


  void virtualized_timers_init();
  uint8_t virtualized_timers_start(uint16_t duration, timer_type_t type, timer_cbt callback);
  void virtualized_timers_stop(uint8_t id);

#endif /* VIRTUALIZED_TIMERS_H_ */
