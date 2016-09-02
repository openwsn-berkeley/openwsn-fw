/**
 * Author: Sven Akkermans (sven.akkermans@cs.kuleuven.be)
 * Date:   September 2015
 * Description: atmega128rfa1-specific definition of the "bsp_timer" bsp module.
 */

#include "bsp_timer.h"
#include "board.h"

//=========================== define ==========================================

//=========================== variables =======================================

typedef struct {
	bsp_timer_cbt    cb;
	PORT_TIMER_WIDTH last_compare_value;
	PORT_TIMER_WIDTH delay_ticks;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
\brief Initialize this module.

This functions starts the timer, i.e. the counter increments, but doesn't set
any compare registers, so no interrupt will fire.
 */
void bsp_timer_init(){
	memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));	// clear local variables

	SCIRQM &= ~(1<<IRQMCP1);		// disable interrupt
	SCIRQS |= (1<<IRQMCP1);		   // reset pending interrupt

	//Datasheet is vague/wrong: the symbol counter always runs at 62.5KHz
	SCCR0 |= (1<<SCEN); // enable symbol counter
	SCCR0 &= ~(1 << SCCKSEL); // 62.5KHz clock from 16MHz clock
//	SCCR0 |= (1<<SCCKSEL); 	// 62.5KHz clock from RTC clock

	SCCR0 &= ~(1 << SCCMP1); //Absolute compare
	SCCR0 &= ~(1<<SCTSE); //no automatic timestamping
	SCCR1 = 0; // no backoff slot counter

//	ASSR |= (1<<AS2); //For RTC clock

	SCOCR1HH = SCOCR1HL = SCOCR1LH = 0; //Set compare registers
	SCOCR1LL = 0;

	while(SCSR & (1<<SCBSY)); 	// wait for register writes
}

/**
\brief Register a callback.

\param cb The function to be called when a compare event happens.
 */
void bsp_timer_set_callback(bsp_timer_cbt cb) {
	bsp_timer_vars.cb   = cb;
}

/**
\brief Reset the timer.

This function does not stop the timer, it rather resets the value of the
counter, and cancels a possible pending compare event.
 */
void bsp_timer_reset(){
	SCOCR1HH = SCOCR1HL = SCOCR1LH = 0;	//Reset Compare
	SCOCR1LL = 0;

	SCIRQM &= ~(1<<IRQMCP1); // disable interrupt
	SCIRQS |= (1<<IRQSCP1);	//Clear compare match IRQ

	SCCNTHH = SCCNTHL = SCCNTLH = 0;	   // reset timer
	SCCNTLL = 0;

	bsp_timer_vars.last_compare_value =  0;	// record last timer compare value
}

/**
\brief Schedule the callback to be called in some specified time.

The delay is expressed relative to the last compare event. It doesn't matter
how long it took to call this function after the last compare, the timer will
expire precisely delayTicks after the last one.

The only possible problem is that it took so long to call this function that
the delay specified is shorter than the time already elapsed since the last
compare. In that case, this function triggers the interrupt to fire right away.

This means that the interrupt may fire a bit off, but this inaccuracy does not
propagate to subsequent timers.

\param delayTicks Number of ticks before the timer expired, relative to the
                  last compare event.
 */
void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks){
	PORT_TIMER_WIDTH newCompareValue;
	PORT_TIMER_WIDTH temp_last_compare_value;
	PORT_TIMER_WIDTH current_value;

	delayTicks = (delayTicks + (TIMER_PRESCALE/2)) * TIMER_PRESCALE;  //Counter runs at 62.5KHz  and we want 32KHz = 1s

	bsp_timer_vars.delay_ticks = delayTicks; // necessary to recover from overflow

	temp_last_compare_value = bsp_timer_vars.last_compare_value;
	newCompareValue      =  bsp_timer_vars.last_compare_value + delayTicks;
	bsp_timer_vars.last_compare_value   =  newCompareValue;

	current_value = bsp_timer_get_currentValue();

	if (current_value > temp_last_compare_value && delayTicks < current_value - temp_last_compare_value) {
		// we're already too late, schedule the ISR right now manually
		// setting the interrupt flag triggers an interrupt
		SCIRQS |= (1<<IRQSCP1);

	} else {
		SCOCR1HH  =  (uint8_t)(newCompareValue>>24); 		//  normal case, have timer expire at newCompareValue
		SCOCR1HL  =  (uint8_t)(newCompareValue>>16);
		SCOCR1LH  =  (uint8_t)(newCompareValue>>8);
		SCOCR1LL  =  (uint8_t)(newCompareValue);
	}
	SCIRQM |= (1<<IRQMCP1);		// enable interrupts
}

/**
\brief Cancel a running compare.
 */
void bsp_timer_cancel_schedule(){
	SCIRQM &= ~(1<<IRQMCP1); // disable interrupt
	SCIRQS |= (1<<IRQSCP1);	//Clear compare match IRQ

	SCOCR1HH = SCOCR1HL = SCOCR1LH = 0; 	//reset compare registers
	SCOCR1LL = 0;
}

/**
\brief Return the current value of the timer's counter.

\returns The current value of the timer's counter.
 */
PORT_TIMER_WIDTH   bsp_timer_get_currentValue(){
	PORT_TIMER_WIDTH retval = SCCNTLL;
	retval |= (PORT_TIMER_WIDTH)SCCNTLH << 8;
	retval |= (PORT_TIMER_WIDTH)SCCNTHL << 16;
	retval |= (PORT_TIMER_WIDTH)SCCNTHH << 24;

	retval = (retval + (TIMER_PRESCALE/2))/ TIMER_PRESCALE; //62.5 KHz clock prescale to 32KHz
	return retval;
}

//=========================== private =========================================

//=========================== interrupt handlers ===============================
kick_scheduler_t   bsp_timer_isr(){
	// call the callback
	bsp_timer_vars.cb();
	// kick the OS
	return KICK_SCHEDULER;
}

kick_scheduler_t   bsp_timer_overflow_isr(){
	uint16_t passed_time;
	uint16_t adjusted_delay_ticks;

	// the timer has overflown, we need to set the bsp timer to fire in the previously scheduled delay_ticks
	// minus the time that has expired before overflow.
	// 2^32 = 4294967296
	passed_time = 4294967296 - bsp_timer_vars.last_compare_value;
	adjusted_delay_ticks = bsp_timer_vars.delay_ticks - passed_time;
	// call the callback
	bsp_timer_scheduleIn(adjusted_delay_ticks);
	// kick the OS
	return KICK_SCHEDULER;
}
