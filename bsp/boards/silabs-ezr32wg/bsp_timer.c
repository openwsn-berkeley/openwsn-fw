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
#include "em_letimer.h"
#include "em_chip.h"


//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
	bsp_timer_cbt cb;
	PORT_TIMER_WIDTH last_compare_value;
        bool initiated;

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
void bsp_timer_init(void) {
        
	// clear local variables
	memset(&bsp_timer_vars, 0, sizeof(bsp_timer_vars_t));

	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
        CMU_ClockEnable(cmuClock_CORELE, true);
        // enable peripheral Sleeptimer
	
	/* Enable clock for TIMER0 module */
	CMU_ClockEnable(cmuClock_LETIMER0, true);
  
        /* Enable clock for GPIO module */
	CMU_ClockEnable(cmuClock_GPIO, true);
        
	/* Set CC1 location 3 pin (PD2) as output */
	//GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 0);
        GPIO_PinModeSet(gpioPortD, 6, gpioModePushPull, 0);
        GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 0);


	     const LETIMER_Init_TypeDef letimerInit = 
  {
  .enable         = true,                   /* Start counting when init completed. */
  .debugRun       = false,                  /* Counter shall not keep running during debug halt. */
  .rtcComp0Enable = false,                  /* Don't start counting on RTC COMP0 match. */
  .rtcComp1Enable = false,                  /* Don't start counting on RTC COMP1 match. */
  .comp0Top       = false,                   /* Load COMP0 register into CNT when counter underflows. COMP0 is used as TOP */
  .bufTop         = false,                  /* Don't load COMP1 into COMP0 when REP0 reaches 0. */
  .out0Pol        = 0,                      /* Idle value for output 0. */
  .out1Pol        = 0,                      /* Idle value for output 1. */
  .ufoa0          = letimerUFOANone,      
  .ufoa1          = letimerUFOANone,      
  .repMode        = letimerRepeatFree       /* Count until stopped */
  };
  
      /* Initialize LETIMER */
      LETIMER_Init(LETIMER0, &letimerInit); 
      LETIMER0->REP0 = 1 ;
      LETIMER0->REP1 = 1 ;
  
      bsp_timer_vars.initiated = false;
  
      /*enable timer0*/
      LETIMER_IntEnable(LETIMER0, LETIMER_IF_COMP1);
      /* Enable TIMER0 interrupt vector in NVIC */
      NVIC_EnableIRQ(LETIMER0_IRQn);
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
void bsp_timer_reset(void) {
	//reset compare
	
	bsp_timer_vars.initiated=FALSE;
	LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);
        bsp_timer_vars.initiated=false;
	// record last timer compare value
	bsp_timer_vars.last_compare_value = 0;
}
/*
* @brief TIMER0_IRQHandler
* Interrupt Service Routine TIMER0 Interrupt Line
*****************************************************************************/
void LETIMER0_IRQHandler(void)
{
    LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1);
    
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
        
	if (!bsp_timer_vars.initiated){
		//as the timer runs forever the first time it is turned on has a weird value
		bsp_timer_vars.last_compare_value=0; //SleepModeTimerCountGet();
		bsp_timer_vars.initiated=TRUE;
	}

	temp_last_compare_value = bsp_timer_vars.last_compare_value;
        
        /* Since the CNT is goes from FF to 0, the future would be lastcomparevalue - delayTicks*/
	newCompareValue = bsp_timer_vars.last_compare_value - delayTicks;
	bsp_timer_vars.last_compare_value = newCompareValue;
        
	if (delayTicks < (temp_last_compare_value - bsp_timer_get_currentValue())) {
               
                LETIMER_IntSet(LETIMER0, LETIMER_IFS_COMP1);

	} else {
		// this is the normal case, have timer expire at newCompareValue
		LETIMER_CompareSet(LETIMER0, 1, newCompareValue);
	}

}

/**
 \brief Cancel a running compare.
 */
void bsp_timer_cancel_schedule(void) {

        LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
        LETIMER_Enable(LETIMER0, false);
}

/**
 \brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
 */
PORT_TIMER_WIDTH bsp_timer_get_currentValue(void) {
	return LETIMER_CounterGet(LETIMER0); //SleepModeTimerCountGet();
}

//=========================== private =========================================

void bsp_timer_isr_private(void) {
	debugpins_isr_set();
	bsp_timer_isr();
	debugpins_isr_clr();
}

//=========================== interrupt handlers ==============================

kick_scheduler_t bsp_timer_isr(void) {

	// call the callback
	bsp_timer_vars.cb();
	// kick the OS

	return KICK_SCHEDULER;
}

