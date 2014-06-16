/* === INCLUDES ============================================================ */

#include "compiler.h"
#include "bsp_timer.h"
#include "tc.h"
#include "tc_interrupt.h"
#include "debugpins.h"

/* === MACROS ============================================================== */
/* Timer module used for BSP Timer */
#define BSP_TIMER     TC3

/* Maximum Timer period value */
#define TIMER_PERIOD UINT16_MAX

/* === Typedef ============================================================= */
/* Structure to hold the bsp timer related functionalities */
typedef struct
{
	bsp_timer_cbt cb;
	PORT_TIMER_WIDTH last_compare_value;
} bsp_timer_vars_t;

/* === GLOBALS ============================================================= */
/* This variables keeps information about bsp timer and callbacks */
bsp_timer_vars_t bsp_timer_vars;

/* Timer Structure to hold the BSP Timer instance */
struct tc_module tc_instance;

/* === PROTOTYPE ============================================================= */
/* Call back handler for capture compare */
static void tc_cca0_callback(struct tc_module *const module_instance);

/* 
 * @brief bsp_timer_init  will Initialize the BSP Timer with default configuration
 *
 * @param None
 */
void bsp_timer_init(void)
{
	struct tc_config timer_config;
	/* clear variables */
	memset(&bsp_timer_vars, 0, sizeof(bsp_timer_vars_t));
    
	/* set the timer structure to default value */
	tc_get_config_defaults(&timer_config);
		
#if 1 //Testing
    /* RTC must be enabled, before using 32.768KHz as a input clock for timer */
	timer_config.clock_source               = GCLK_GENERATOR_0;
	timer_config.clock_prescaler            = TC_CLOCK_PRESCALER_DIV1024;
#endif

#if 0 //Testing
    /* GCLK_GENERATOR_2 must be enabled, before using it as a input clock for timer */
	timer_config.clock_source               = GCLK_GENERATOR_2;
	timer_config.clock_prescaler            = TC_CLOCK_PRESCALER_DIV1;
#endif		
	
	timer_config.counter_size                    = TC_COUNTER_SIZE_16BIT;
	timer_config.wave_generation                 = TC_CTRLA_WAVEGEN_MFRQ;
	timer_config.counter_16_bit.compare_capture_channel[0] = TIMER_PERIOD;
	timer_config.counter_16_bit.compare_capture_channel[1] = 0;
	timer_config.run_in_standby             = true;
	tc_init(&tc_instance, BSP_TIMER, &timer_config);
	
#if 0 //Testing
	struct tc_events tc_event;
	memset(&tc_event, 0, sizeof(struct tc_events));
	tc_event.on_event_perform_action = true;
	tc_event.event_action = tc_event_action_increment_counter;
	tc_enable_events(&tc_instance, &tc_event);
#endif	
	
	tc_cont_sync_enable(&tc_instance, BSP_TIMER);
	tc_register_callback(&tc_instance, tc_cca0_callback, TC_CALLBACK_CC_CHANNEL0);	
	tc_enable(&tc_instance);
}

/* 
 * @brief bsp_timer_set_callback  Register the call back for bsp timer
 *
 * @param cb function pointer to the bsp_timer call back function
 */
void bsp_timer_set_callback(bsp_timer_cbt cb)
{
	bsp_timer_vars.cb = cb;	
}

/* 
 * @brief tc_cca0_callback  call back function for bsp_timer called from ISR
 *
 * @param module_instance - timer module instance from interrupt
 */
static void tc_cca0_callback(struct tc_module *const module_instance)
{
	/* Capture Compare 0 callback */
	debugpins_isr_set();
	if(bsp_timer_vars.cb != NULL)
	{
		bsp_timer_vars.cb();
	}	
	debugpins_isr_clr();
}

/* 
 * @brief bsp_timer_reset  reset the bsp timer to default but do not stop the timer
 *
 * @param None
 */
void bsp_timer_reset(void)
{
	//disable will clears the compare interrupt
	tc_disable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);	
	tc_set_compare_value(&tc_instance, TC_COMPARE_CAPTURE_CHANNEL_0, TIMER_PERIOD);	
	// reset timer -- set counter to 0
	tc_set_count_value(&tc_instance, 0);
	// record last timer compare value
	bsp_timer_vars.last_compare_value =  0;
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
void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks)
{
   PORT_TIMER_WIDTH newCompareValue;
   PORT_TIMER_WIDTH temp_last_compare_value;
   PORT_TIMER_WIDTH current_value;
   
   temp_last_compare_value = bsp_timer_vars.last_compare_value;   
   newCompareValue = bsp_timer_vars.last_compare_value+delayTicks+1;
   bsp_timer_vars.last_compare_value = newCompareValue;
   current_value = (PORT_TIMER_WIDTH)tc_get_count_value(&tc_instance);
   
   if (delayTicks < (current_value-temp_last_compare_value))
   {
	   /* This needs to be fixed */
	   tc_set_interrupt(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
   }
   else
   { 	   
       /* this is the normal case, have timer expire at newCompareValue */
	   tc_set_compare_value(&tc_instance, TC_COMPARE_CAPTURE_CHANNEL_0, newCompareValue);
	   tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
   }	
}

/* 
 * @brief bsp_timer_cancel_schedule  Cancel the running compare
 *
 * @param None
 */
void bsp_timer_cancel_schedule(void)
{
	tc_disable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
    tc_set_compare_value(&tc_instance, TC_COMPARE_CAPTURE_CHANNEL_0, TIMER_PERIOD);   
}

/* 
 * @brief bsp_timer_get_currentValue  Get current timer counter value
 *
 * @param PORT_TIMER_WIDTH return current bsp timer counter value
 */
PORT_TIMER_WIDTH bsp_timer_get_currentValue(void)
{
	return ((PORT_TIMER_WIDTH)tc_get_count_value(&tc_instance));
}
