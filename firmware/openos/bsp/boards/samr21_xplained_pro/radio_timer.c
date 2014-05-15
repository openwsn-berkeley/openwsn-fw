#include "compiler.h"
#include "radiotimer.h"
#include "board_info.h"
#include "leds.h"
#include "tc.h"
#include "tc_interrupt.h"
#include "debugpins.h"

#define TIMER1     TC4
#define TIMER_PERIOD UINT16_MAX

struct tc_config timer2_config;
struct tc_module tc2_instance;

static void tc2_ovf_callback(struct tc_module *const module_instance);
static void tc2_cca0_callback(struct tc_module *const module_instance);
static void tc2_cca1_callback(struct tc_module *const module_instance);

typedef struct {
	radiotimer_compare_cbt    overflow_cb;
	radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

void radiotimer_init(void)
{
    // clear local variables
    memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));

	tc_get_config_defaults(&timer2_config);
	/* Before that XOSC - 32.768KHz must be enabled */
	timer2_config.clock_source               = GCLK_GENERATOR_2;
	timer2_config.counter_16_bit.value       = 0;
	timer2_config.wave_generation = TC_CTRLA_WAVEGEN_MFRQ;
	timer2_config.counter_16_bit.compare_capture_channel[0] = TIMER_PERIOD;
	timer2_config.counter_16_bit.compare_capture_channel[1] = TIMER_PERIOD;
	timer2_config.run_in_standby = true;
	tc_init(&tc2_instance, TIMER1, &timer2_config);		
	tc_cont_sync_enable(&tc2_instance, TIMER1);
	
	tc_register_callback(&tc2_instance, tc2_ovf_callback,  TC_CALLBACK_OVERFLOW);
	tc_register_callback(&tc2_instance, tc2_cca0_callback, TC_CALLBACK_CC_CHANNEL0);
	tc_register_callback(&tc2_instance, tc2_cca1_callback, TC_CALLBACK_CC_CHANNEL1);
	
	tc_enable(&tc2_instance);
	
}

static void tc2_ovf_callback(struct tc_module *const module_instance)
{
 //No handle
}

static void tc2_cca0_callback(struct tc_module *const module_instance)
{
 debugpins_isr_set();
 if (radiotimer_vars.overflow_cb != NULL)
 {
	 // call the callback
	 radiotimer_vars.overflow_cb();	 	 
 }
 debugpins_isr_clr();	
}

static void tc2_cca1_callback(struct tc_module *const module_instance)
{
  debugpins_isr_set();
 if (radiotimer_vars.compare_cb != NULL) 
 {
	 radiotimer_vars.compare_cb();
 }	
 debugpins_isr_clr();
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb)
{
 radiotimer_vars.overflow_cb    = cb;  
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb)
{
 radiotimer_vars.compare_cb     = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb)
{
 while(1);
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb)
{
 while(1);
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period)
{
 tc_disable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL0);  
 tc_set_count_value(&tc2_instance, 0);  
 tc_set_top_value(&tc2_instance, period);
 tc_enable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL0);
}

PORT_RADIOTIMER_WIDTH radiotimer_getValue(void)
{
	PORT_RADIOTIMER_WIDTH time_val;
	dbg_pin1_set();	
	time_val = tc_get_count_value(&tc2_instance);
    dbg_pin1_clr();    
    return (time_val);
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period)
{
 tc_disable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL0);
 tc_set_top_value(&tc2_instance, period);
 tc_enable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL0);
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod(void)
{
 return ((PORT_RADIOTIMER_WIDTH)tc_get_capture_value(&tc2_instance, TC_CALLBACK_CC_CHANNEL0));
}

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset)
{
  tc_disable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL1);
  tc_set_compare_value(&tc2_instance, TC_COMPARE_CAPTURE_CHANNEL_1, offset);
  tc_enable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL1);
}

void radiotimer_cancel(void)
{  
  tc_disable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL1);
  tc_set_compare_value(&tc2_instance, TC_COMPARE_CAPTURE_CHANNEL_1, TIMER_PERIOD);
}

inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(void)
{
 return((PORT_RADIOTIMER_WIDTH)tc_get_count_value(&tc2_instance));
}



