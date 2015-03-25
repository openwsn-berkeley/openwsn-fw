/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "bsp_timer" bsp module.
 */

#include <headers/hw_ints.h>
#include <headers/hw_memmap.h>
#include <headers/hw_smwdthrosc.h>

#include "string.h"
#include "bsp_timer.h"
#include "board.h"
#include "debug.h"
#include "interrupt.h"
#include "gptimer.h"
#include "debugpins.h"

//=========================== defines =========================================

#define BSP_TIMER_32MHZ_TICS_PER_32KHZ_TIC  ( 976 )

#define GPTIMER0_STALL                      ( 0x4003000C )

//=========================== variables =======================================

typedef struct {
	bsp_timer_cbt cb;
} bsp_timer_vars_t;

static bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================

void bsp_timer_isr_private(void);

//=========================== public ==========================================

/**
 \brief Initialize this module.

 This functions starts the timer, i.e. the counter increments, but doesn't set
 any compare registers, so no interrupt will fire.
 */
void bsp_timer_init() {
	// Clear local variables
	memset(&bsp_timer_vars, 0, sizeof(bsp_timer_vars_t));

	// Configure the timer as a 32-bit timer
	TimerConfigure(GPTIMER0_BASE, GPTIMER_CFG_ONE_SHOT);

	// Stall the timer while debugging
	HWREG(GPTIMER0_STALL) |= 0x02;
}

/**
 \brief Register a callback.

 \param cb The function to be called when a compare event happens.
 */
void bsp_timer_set_callback(bsp_timer_cbt cb) {
	bsp_timer_vars.cb = cb;
}

/**
 \brief Reset the timer.

 This function does not stop the timer, it rather resets the value of the
 counter, and cancels a possible pending compare event.
 */
void bsp_timer_reset() {
    // Disable the timer interrupt
    TimerIntDisable(GPTIMER0_BASE, INT_TIMER0A);

    // Disable the timer
	TimerDisable(GPTIMER0_BASE, GPTIMER_CFG_ONE_SHOT);
}

void bsp_timer_suspend(void) {
    // Disable the timer interrupt
    TimerIntDisable(GPTIMER0_BASE, INT_TIMER0A);

    // Disable the timer
    TimerDisable(GPTIMER0_BASE, GPTIMER_BOTH);

    // Stall the timer while debugging
    HWREG(GPTIMER0_STALL) |= 0x02;
}

void bsp_timer_wakeup(PORT_TIMER_WIDTH elapsed) {
    PORT_TIMER_WIDTH current;
    int32_t remaining;

    // Scale up from 32.768 kHz to 32 MHz
    elapsed *= BSP_TIMER_32MHZ_TICS_PER_32KHZ_TIC;

    // Get the remaining number of ticks
    // Remember that this is a down-counter!
    current = bsp_timer_get_currentValue();

    // The remaining is the current minus the elapsed
    remaining = current - elapsed;

    // If there is nothing remaining
    if (remaining <= BSP_TIMER_32MHZ_TICS_PER_32KHZ_TIC) {
        // Enable the timer interrupt
        IntPendSet(INT_TIMER0A);
    } else {
        // Set the timer compare value to the remaining value
        TimerLoadSet(GPTIMER0_BASE, GPTIMER_A, (uint32_t) remaining);

        // Enable the interrupt
        TimerIntEnable(GPTIMER0_BASE, GPTIMER_TIMA_TIMEOUT);

        // Enable the timer
        TimerEnable(GPTIMER0_BASE, GPTIMER_BOTH);
    }
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
void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks) {
    // Scale up from 32.768 kHz to 32 MHz
    delayTicks *= BSP_TIMER_32MHZ_TICS_PER_32KHZ_TIC;

    // Set the timer compare value
    TimerLoadSet(GPTIMER0_BASE, GPTIMER_A, delayTicks);

    // Register the timer interrupt and enable it
    TimerIntRegister(GPTIMER0_BASE, GPTIMER_BOTH, bsp_timer_isr_private);
    TimerIntEnable(GPTIMER0_BASE, GPTIMER_TIMA_TIMEOUT);

    // Enable the timer interrupt
    IntEnable(INT_TIMER0A);

    // Enable the timer
    TimerEnable(GPTIMER0_BASE, GPTIMER_BOTH);
}

/**
 \brief Cancel a running compare.
 */
void bsp_timer_cancel_schedule() {
    // Disable the timer interrupt
    TimerIntDisable(GPTIMER0_BASE, INT_TIMER0A);

    // Disable the timer
    TimerDisable(GPTIMER0_BASE, GPTIMER_BOTH);
}

/**
 \brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
 */
PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
	return TimerLoadGet(GPTIMER0_BASE, GPTIMER_A);
}

/**
 \brief Return the remaining value of the timer's counter.

 \returns The value that remains of the timer's counter.
 */
PORT_TIMER_WIDTH bsp_timer_get_remainingValue() {
    PORT_TIMER_WIDTH current;

    current  = TimerValueGet(GPTIMER0_BASE, GPTIMER_A);
    current /= BSP_TIMER_32MHZ_TICS_PER_32KHZ_TIC;

    return  current;
}

//=========================== private =========================================

void bsp_timer_isr_private(void) {
	debugpins_isr_set();

	// Clear the pending interrupt
	TimerIntClear(GPTIMER0_BASE, GPTIMER_TIMA_TIMEOUT);

	// Execute the interrupt
	bsp_timer_isr();

	debugpins_isr_clr();
}

//=========================== interrupt handlers ==============================

kick_scheduler_t bsp_timer_isr() {
    // Execute the callback
    if (bsp_timer_vars.cb != NULL) {
        bsp_timer_vars.cb();
    }

	// Don't kick the OS
	return DO_NOT_KICK_SCHEDULER;
}
