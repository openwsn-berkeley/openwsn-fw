/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: EZR32WG-specific definition of the "bsp_timer" bsp module.
 */

#include "string.h"
#include "bsp_timer.h"
#include "board.h"
#include "debugpins.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_timer.h"

//#define TOP 65535


//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
	bsp_timer_cbt cb;
	PORT_TIMER_WIDTH last_compare_value;

} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================
void bsp_timer_isr_private(void);
//=========================== public ==========================================

/**
 \brief Initialize this module.

 This functions starts the timer, i.e. the counter increments, but doesn't set
 any compare registers, so no interrupt will fire.
 */
void bsp_timer_init() {
        
	// clear local variables
	memset(&bsp_timer_vars, 0, sizeof(bsp_timer_vars_t));

	// enable peripheral Sleeptimer
	/* Enable clock for GPIO module */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Enable clock for TIMER0 module */
	CMU_ClockEnable(cmuClock_TIMER0, true);

	/* Set CC1 location 3 pin (PD2) as output */
	GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 0);

	/*enable timer0*/
	TIMER_IntEnable(TIMER0, TIMER_IEN_CC1);

	  /* Select CC channel parameters */
	   TIMER_InitCC_TypeDef timerCCInit =
	   {
	     .cufoa      = timerOutputActionNone,
	     .cofoa      = timerOutputActionNone,
	     .cmoa       = timerOutputActionToggle,
	     .mode       = timerCCModeCompare,
	     .filter     = true,
	     .prsInput   = false,
	     .coist      = false,
	     .outInvert  = false,
	   };

	/* Configure CC channel 0 */
	TIMER_InitCC(TIMER0, 1, &timerCCInit);

	/* Route CC1 to location 3 (PD2) and enable pin */
	TIMER0->ROUTE |= (TIMER_ROUTE_CC1PEN | TIMER_ROUTE_LOCATION_LOC3);

	/* Set Top Value */
//	TIMER_TopSet(TIMER0, TOP);
        
        /* Set Compare Value */
        //  TIMER_CompareSet(TIMER0, 1, 13672); 

	/* Select timer parameters */
	TIMER_Init_TypeDef timerInit =
	{
	  .enable     = true,
	  .debugRun   = false,
	  .prescale   = timerPrescale1024,
	  .clkSel     = timerClkSelHFPerClk,
	  .fallAction = timerInputActionNone,
	  .riseAction = timerInputActionNone,
	  .mode       = timerModeUp,
	  .dmaClrAct  = false,
	  .quadModeX4 = false,
	  .oneShot    = false,
	  .sync       = false,
	};

	/* Configure timer */
	TIMER_Init(TIMER0, &timerInit);

}

/**
 \brief Register a callback.

 \param cb The function to be called when a compare event happens.
 */
void bsp_timer_set_callback(bsp_timer_cbt cb) {
	bsp_timer_vars.cb = cb;
	/* Enable TIMER0 interrupt vector in NVIC */
	NVIC_EnableIRQ(TIMER0_IRQn);
}

/**
 \brief Reset the timer.

 This function does not stop the timer, it rather resets the value of the
 counter, and cancels a possible pending compare event.
 */
void bsp_timer_reset() {
	// reset compare
	TIMER_CompareSet(TIMER0, 1, 0);

	//enalbe compare interrupt
	// reset timer
    //bsp_timer_vars.initiated=FALSE;
	TIMER_IntClear(TIMER0, TIMER_IFC_CC0);
	TIMER_CounterSet(TIMER0, 0);

	// record last timer compare value
	bsp_timer_vars.last_compare_value = 0;
}
/*
* @brief TIMER0_IRQHandler
* Interrupt Service Routine TIMER0 Interrupt Line
*****************************************************************************/
void TIMER0_IRQHandler(void)
{

 TIMER_IntClear(TIMER0, TIMER_IFC_CC1);
 
 //debugpins_isr_toggle();

 /* Toggle LED ON/OFF */
 //GPIO_PinOutToggle(5, 7);
 //bsp_timer_isr();
 bsp_timer_isr_private();
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
	PORT_TIMER_WIDTH newCompareValue;
	PORT_TIMER_WIDTH temp_last_compare_value;
       // uint16_t compare_last_value;
	//if (!bsp_timer_vars.initiated){
		//as the timer runs forever the first time it is turned on has a weird value
	//	bsp_timer_vars.last_compare_value=0; //SleepModeTimerCountGet();
	//	bsp_timer_vars.initiated=TRUE;
//	}

	/*enable timer0, if not enabled*/
	//TIMER_IntEnable(TIMER0, TIMER_IEN_CC1);


	temp_last_compare_value = bsp_timer_vars.last_compare_value;

	newCompareValue = bsp_timer_vars.last_compare_value + delayTicks;
	bsp_timer_vars.last_compare_value = newCompareValue;
        //compare_last_value = uint16_t
        
	if (delayTicks < (TIMER_CounterGet(TIMER0) - temp_last_compare_value)) {
               
                TIMER_IntSet(TIMER0, TIMER_IFS_CC1);

	} else {
		// this is the normal case, have timer expire at newCompareValue
		TIMER_CompareSet(TIMER0, 1, newCompareValue);
	}

}

/**
 \brief Cancel a running compare.
 */
void bsp_timer_cancel_schedule() {
	// Disable the Timer0B interrupt.
	//IntDisable(INT_SMTIM);
	//TIMER_CompareSet(TIMER0, 1, 0);
	TIMER_IntDisable(TIMER0, TIMER_IEN_CC1);
        TIMER_Enable(TIMER0, false);
}

/**
 \brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
 */
PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
	return TIMER_CounterGet(TIMER0); //SleepModeTimerCountGet();
}

//=========================== private =========================================

void bsp_timer_isr_private(void) {
	debugpins_isr_set();
	//TIMER_IntDisable(TIMER0, TIMER_IEN_CC1);
	bsp_timer_isr();
	debugpins_isr_clr();
}

//=========================== interrupt handlers ==============================

kick_scheduler_t bsp_timer_isr() {

	// call the callback
	bsp_timer_vars.cb();
	// kick the OS

	return KICK_SCHEDULER;
}

