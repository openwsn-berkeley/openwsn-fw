/**
 * Author: Sven Akkermans (sven.akkermans@cs.kuleuven.be)
 * Date:   September 2015
 * Description: atmega128rfa1-specific definition of the "bsp_timer" bsp module.
 */

#include "bsp_timer.h"
#include "board.h"
#include "board_info.h"

//=========================== define ==========================================

//Macros for watchdog timer (modified from wdt.h)
#define wdt_enable_int_only(value)   \
		__asm__ __volatile__ (  \
				"in __tmp_reg__,__SREG__" "\n\t"    \
				"cli" "\n\t"    \
				"wdr" "\n\t"    \
				"sts %0,%1" "\n\t"  \
				"out __SREG__,__tmp_reg__" "\n\t"   \
				"sts %0,%2" "\n\t" \
				: /* no outputs */  \
		: "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
		  "r" (_BV(_WD_CHANGE_BIT) | _BV(WDE)), \
		  "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) | \
				  _BV(WDIE) | (value & 0x07)) ) \
				  : "r0"  \
		)
//=========================== variables =======================================

typedef struct {
	bsp_timer_cbt    cb;
	PORT_TIMER_WIDTH time_set;
	PORT_TIMER_WIDTH time_left;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
\brief Initialize this module.

This functions starts the timer, i.e. the counter increments, but doesn't set
any compare registers, so no interrupt will fire.
 */
//The watchdog timer can count up to a precision of 16ms and arbitrarily high.
void bsp_timer_init(){
	// clear local variables
	memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));

	MCUSR &=~(1<<WDRF);	//  Clear startup bit and disable the wdt, whether or not it will be used.
	WDTCSR = (0<<WDIE);	// Disable watchdog timeout interrupt enable
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
	wdt_reset();
	// record last timer compare value
	bsp_timer_vars.time_left =  0;
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
	if(bsp_timer_vars.time_set == 0){
		bsp_timer_vars.time_set = delayTicks;
	} else {
		delayTicks = bsp_timer_vars.time_set - delayTicks; //regain remaining delayticks.
	}
	if(delayTicks <= 2048){ //wdt can't count this small, trigger immediate
		bsp_timer_isr();
	} else{
		uint32_t countedCycles = 2048; //smallest possible cycles
		int wdt_count = 0;
		while(countedCycles*2 <= delayTicks && countedCycles != 1048576){ //1048576 is the max time the wdt can count
			wdt_count++;
			countedCycles = countedCycles*2;
		}
		bsp_timer_vars.time_left = delayTicks - countedCycles;

		wdt_enable_int_only(wdt_count); //Macro sets things up for a reset
		WDTCSR |= (1 << WDIE);
	}
}

/**
\brief Cancel a running compare.
 */
void bsp_timer_cancel_schedule(){
	bsp_timer_vars.time_set = 0;
	bsp_timer_vars.time_left = 0;
	wdt_disable();
}

/**
\brief Return the current value of the timer's counter.

\returns The current value of the timer's counter.
 */
PORT_TIMER_WIDTH   bsp_timer_get_currentValue(){
	return bsp_timer_vars.time_set-bsp_timer_vars.time_left;
}

//=========================== private =========================================

//=========================== interrupt handlers ===============================
kick_scheduler_t   bsp_timer_isr(){
	bsp_timer_cancel_schedule();
	// call the callback
	bsp_timer_vars.cb();
	// kick the OS
	return 1;
}

