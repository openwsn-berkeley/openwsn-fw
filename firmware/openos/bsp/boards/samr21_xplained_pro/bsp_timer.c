
#include "compiler.h"
#include "bsp_timer.h"
#include "tc.h"
#include "tc_interrupt.h"
#include "debugpins.h"

#define BSP_TIMER     TC3
#define TIMER_PERIOD UINT16_MAX
typedef struct
{
	bsp_timer_cbt cb;
	PORT_TIMER_WIDTH last_compare_value;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

struct tc_config timer_config;
struct tc_module tc_instance;

void configure_tc_callbacks(void);
static void tc_ovf_callback(struct tc_module *const module_instance);
static void tc_cca0_callback(struct tc_module *const module_instance);
static void tc_cca1_callback(struct tc_module *const module_instance);


void bsp_timer_init(void)
{
	// clear local variables
	memset(&bsp_timer_vars, 0, sizeof(bsp_timer_vars_t));

	tc_get_config_defaults(&timer_config);
	/* Before that RTC must be enabled */
	timer_config.clock_source               = GCLK_GENERATOR_0;
	timer_config.counter_size               = TC_COUNTER_SIZE_16BIT;
	timer_config.wave_generation                 = TC_CTRLA_WAVEGEN_MFRQ;
	timer_config.counter_16_bit.compare_capture_channel[0] = TIMER_PERIOD;
	timer_config.counter_16_bit.compare_capture_channel[1] = TIMER_PERIOD;
	timer_config.run_in_standby             = true;
	tc_init(&tc_instance, BSP_TIMER, &timer_config);
	
	struct tc_events tc_event;
	memset(&tc_event, 0, sizeof(struct tc_events));
	tc_event.on_event_perform_action = true;
	tc_event.event_action = TC_EVENT_ACTION_INCREMENT_COUNTER;
	tc_enable_events(&tc_instance, &tc_event);
	
	
	tc_cont_sync_enable(&tc_instance, BSP_TIMER);
	
	tc_register_callback(&tc_instance,tc_ovf_callback,TC_CALLBACK_OVERFLOW);
	tc_register_callback(&tc_instance, tc_cca0_callback, TC_CALLBACK_CC_CHANNEL0);
	tc_register_callback(&tc_instance, tc_cca1_callback, TC_CALLBACK_CC_CHANNEL1);
	
	tc_enable(&tc_instance);
	tc_readreq_set(&tc_instance);	
}

void bsp_timer_set_callback(bsp_timer_cbt cb)
{
	bsp_timer_vars.cb = cb;	
}

static void tc_ovf_callback(struct tc_module *const module_instance)
{
	//overflow Callback
}

static void tc_cca0_callback(struct tc_module *const module_instance)
{
	//Capture Compare 0 callback
}

static void tc_cca1_callback(struct tc_module *const module_instance)
{
   //timer capture compare 1 callback
   debugpins_isr_set();
   bsp_timer_isr();
   debugpins_isr_clr();
}


void bsp_timer_reset(void)
{
	// reset compare --- set compare value to 0 ,1
	tc_set_top_value(&tc_instance, TIMER_PERIOD);
	tc_set_compare_value(&tc_instance, TC_COMPARE_CAPTURE_CHANNEL_1, TIMER_PERIOD);
	
	//disable will clears the compare interrupt
	tc_disable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
	tc_disable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
	
	// reset timer -- set counter to 0
	tc_set_count_value(&tc_instance, 0);
	//tc_readreq_set(&tc_instance);
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
   
   newCompareValue = bsp_timer_vars.last_compare_value+delayTicks;
   bsp_timer_vars.last_compare_value = newCompareValue;
   current_value = (PORT_TIMER_WIDTH)tc_get_count_value(&tc_instance);
   //tc_readreq_set(&tc_instance);
   if (delayTicks < (current_value-temp_last_compare_value))
   {
	   tc_set_interrupt(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
   }
   else
   { 
       /* this is the normal case, have timer expire at newCompareValue */
	   tc_set_compare_value(&tc_instance, TC_COMPARE_CAPTURE_CHANNEL_1, newCompareValue);
	   //tc_readreq_set(&tc_instance);   
	   /* Clear interrupt flag not handled */
	   tc_disable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);  
	   tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
   }	
}

/* Cancel the running compare */
void bsp_timer_cancel_schedule(void)
{
    tc_set_compare_value(&tc_instance, TC_COMPARE_CAPTURE_CHANNEL_1, TIMER_PERIOD);
	//tc_readreq_set(&tc_instance);
    tc_disable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
}

/* Get the current timer counter value */
PORT_TIMER_WIDTH bsp_timer_get_currentValue(void)
{
	PORT_TIMER_WIDTH timer_count;
	timer_count = (PORT_TIMER_WIDTH)tc_get_count_value(&tc_instance);
	//tc_readreq_set(&tc_instance);
    return timer_count;	
}

kick_scheduler_t bsp_timer_isr(void)
{
   // call the callback
   bsp_timer_vars.cb();
   // kick the OS
   return KICK_SCHEDULER;	
}


