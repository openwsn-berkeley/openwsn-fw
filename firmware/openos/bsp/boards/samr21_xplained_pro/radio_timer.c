/* === INCLUDES ============================================================ */
#include "compiler.h"
#include "radiotimer.h"
#include "board_info.h"
#include "leds.h"
#include "tc.h"
#include "tcc.h"
#include "tc_interrupt.h"
#include "debugpins.h"
#include "events.h"

/* === MACROS ============================================================== */
/* Timer Module used for Radio */
#define TIMER1     TC4

/* Maximum timer period value */
#define TIMER_PERIOD UINT16_MAX

/* Use the DFLL Clock for Radio Timer */
#define RADIOTIMER_USE_DFLL_CLK  (1)

/* Use the XOSC 32.768KHz for Radio Timer */
#define RADIOTIMER_USE_XOSC		 (0)

/* Use the Event Clock System for Radio Timer */
#define RADIOTIMER_USE_EVENT_CLK (0)

/* === PROTOTYPE ============================================================== */
static void tc2_ovf_callback(struct tc_module *const module_instance);
static void tc2_cca1_callback(struct tc_module *const module_instance);

/* Event system init functions */
void tcc_event_clk_init(void);
void configure_eve_sys(void);

/* === GLOBALS ============================================================= */
/* Timer module instance to store the configuration value of Timer module */
struct tc_module tc2_instance;

typedef struct {
	radiotimer_compare_cbt    overflow_cb;
	radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

/*
 * @brief radiotimer_init  Initialize the radio timer 
 *        with 32.768KHz input clock 
 *
 * @param None
 *
 */
void radiotimer_init(void)
{
	struct tc_config timer2_config;
    /* clear variables */
    memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));

	tc_get_config_defaults(&timer2_config);
	
	
#if (RADIOTIMER_USE_DFLL_CLK)
	timer2_config.clock_source               = GCLK_GENERATOR_0;
	timer2_config.clock_prescaler            = TC_CLOCK_PRESCALER_DIV1024;
#endif

#if (RADIOTIMER_USE_XOSC)
    /* XOSC - 32.768KHz must be enabled before using the XOSC Clock */
	timer2_config.clock_source               = GCLK_GENERATOR_2;
	timer2_config.clock_prescaler            = TC_CLOCK_PRESCALER_DIV1;
#endif	

	timer2_config.counter_16_bit.value       = 0;
	timer2_config.wave_generation = TC_CTRLA_WAVEGEN_MFRQ;
	timer2_config.counter_16_bit.compare_capture_channel[0] = TIMER_PERIOD;
	timer2_config.counter_16_bit.compare_capture_channel[1] = TIMER_PERIOD;
	timer2_config.run_in_standby = true;
	tc_init(&tc2_instance, TIMER1, &timer2_config);

#if (RADIOTIMER_USE_EVENT_CLK)	
	struct tc_events tc_event;
	memset(&tc_event, 0, sizeof(struct tc_events));
	tc_event.on_event_perform_action = true;
	tc_event.event_action = TC_EVENT_ACTION_INCREMENT_COUNTER;
	tc_enable_events(&tc2_instance, &tc_event);
#endif		
			
	tc_cont_sync_enable(&tc2_instance, TIMER1);	
	tc_register_callback(&tc2_instance, tc2_ovf_callback,  TC_CALLBACK_OVERFLOW);
	tc_register_callback(&tc2_instance, tc2_cca1_callback, TC_CALLBACK_CC_CHANNEL1);	
	tc_enable(&tc2_instance);

#if (RADIOTIMER_USE_EVENT_CLK)
	tcc_event_clk_init();
	configure_eve_sys();
#endif	
}

/*
 * @brief tc2_ovf_callback  Timer module overflow or 
 *        match frequency callback 
 *
 * @param module_instance stored timer module instance
 *
 */
static void tc2_ovf_callback(struct tc_module *const module_instance)
{
 debugpins_isr_set();
 if (radiotimer_vars.overflow_cb != NULL)
 {
  // call the callback
  radiotimer_vars.overflow_cb();
 }
 debugpins_isr_clr();
}

/*
 * @brief tc2_cca1_callback  Timer module compare call back 
 *
 * @param module_instance stored timer module instance
 *
 */
static void tc2_cca1_callback(struct tc_module *const module_instance)
{
  debugpins_isr_set();
 if (radiotimer_vars.compare_cb != NULL) 
 {
	 radiotimer_vars.compare_cb();
 }	
 debugpins_isr_clr();
}

/*
 * @brief tc2_cca1_callback  Register the radio timer 
 *        overflow callback 
 *
 * @param cb callback address pointer
 *
 */
void radiotimer_setOverflowCb(radiotimer_compare_cbt cb)
{
 radiotimer_vars.overflow_cb    = cb;  
}

/*
 * @brief radiotimer_setCompareCb  Register the radio timer 
 *        compare callback 
 *
 * @param cb callback address pointer
 *
 */
void radiotimer_setCompareCb(radiotimer_compare_cbt cb)
{
 radiotimer_vars.compare_cb     = cb;
}

/*
 * @brief radiotimer_setStartFrameCb   This should not happen 
 *
 * @param cb callback address pointer
 *
 */
void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb)
{
 while(1);
}

/*
 * @brief radiotimer_setEndFrameCb   This should not happen 
 *
 * @param cb callback address pointer
 *
 */
void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb)
{
 while(1);
}

/*
 * @brief radiotimer_start   Start the radio timer, 
 *        set the radio timer overflow value 
 *
 * @param period set the period value to radio timer
 *
 */
void radiotimer_start(PORT_RADIOTIMER_WIDTH period)
{
 tc_disable_callback(&tc2_instance, TC_CALLBACK_OVERFLOW); 
 tc_disable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL1); 
 tc_set_count_value(&tc2_instance, 0);  
 tc_set_top_value(&tc2_instance, period);
 tc_enable_callback(&tc2_instance, TC_CALLBACK_OVERFLOW);
}

/*
 * @brief radiotimer_getValue   Get the current timer 
 *        counter value
 *
 * @param return PORT_RADIOTIMER_WIDTH counter value
 *
 */
PORT_RADIOTIMER_WIDTH radiotimer_getValue(void)
{
	PORT_RADIOTIMER_WIDTH time_val;
	dbg_pin1_set();	
	time_val = (PORT_RADIOTIMER_WIDTH)tc_get_count_value(&tc2_instance);
    dbg_pin1_clr();    
    return (time_val);
}

/*
 * @brief radiotimer_setPeriod   Set the radio timer period value. 
 *        This will call radio timer overflow
 *
 * @param period set the period value to the radio timer top
 *
 */
void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period)
{
 tc_disable_callback(&tc2_instance, TC_CALLBACK_OVERFLOW);
 tc_set_top_value(&tc2_instance, period);
 tc_enable_callback(&tc2_instance, TC_CALLBACK_OVERFLOW);
}

/*
 * @brief radiotimer_getPeriod   Read the radio timer period value
 *
 * @param return PORT_RADIOTIMER_WIDTH capture value
 *
 */
PORT_RADIOTIMER_WIDTH radiotimer_getPeriod(void)
{
 PORT_RADIOTIMER_WIDTH timer_period_val; 
 timer_period_val = (PORT_RADIOTIMER_WIDTH)tc_get_capture_value(&tc2_instance,\
							TC_COMPARE_CAPTURE_CHANNEL_0);
 return timer_period_val;
}

/*
 * @brief radiotimer_schedule set the next interrupt fire offset from 
 *        current counter value
 *
 * @param offset PORT_RADIOTIMER_WIDTH set the compare value
 *
 */
void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset)
{  
  tc_disable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL1);
  tc_set_compare_value(&tc2_instance, TC_COMPARE_CAPTURE_CHANNEL_1, offset);  
  tc_enable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL1);
}

/*
 * @brief radiotimer_cancel cancel the running radio timer
 *        But this will not stop the radio timer 
 *
 * @param None
 *
 */
void radiotimer_cancel(void)
{  
  tc_disable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL1);
  tc_set_compare_value(&tc2_instance, TC_COMPARE_CAPTURE_CHANNEL_1, TIMER_PERIOD);
}

/*
 * @brief radiotimer_getCapturedTime Read the radio timer current count value
 *
 * @param return PORT_RADIOTIMER_WIDTH count value
 *
 */
inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(void)
{
  return ((PORT_RADIOTIMER_WIDTH)tc_get_count_value(&tc2_instance));
}

/*
 * @brief configure_eve_sys Configure the event system to 
 *        clock the radio timer module
 *
 * @param None
 *
 */
void configure_eve_sys(void)
{
	struct events_resource resource_event;
	struct events_config config;
	
	events_get_config_defaults(&config);
	config.generator      = EVSYS_ID_GEN_TCC0_CNT;
	config.edge_detect    = EVENTS_EDGE_DETECT_BOTH;
	config.path           = EVENTS_PATH_RESYNCHRONIZED;
	config.clock_source   = GCLK_GENERATOR_0;
	events_allocate(&resource_event, &config);
	events_attach_user(&resource_event, EVSYS_ID_USER_TC4_EVU);
	events_attach_user(&resource_event, EVSYS_ID_USER_TC3_EVU);
	
	while (events_is_busy(&resource_event)) {
		/* Wait for channel */
	}
}

/*
 * @brief tcc_event_clk_init Enable the TCC for Event/System Control 
 *
 * @param None
 *
 */
void tcc_event_clk_init(void)
{
	struct tcc_module tcc_instance;
	struct tcc_events tcc_event_config;
	struct tcc_config config_tcc;
	tcc_get_config_defaults(&config_tcc, TCC0);
	config_tcc.counter.clock_source = GCLK_GENERATOR_2;
	config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
	config_tcc.counter.period = 1;
	tcc_init(&tcc_instance, TCC0, &config_tcc);
	tcc_event_config.output_config.generation_selection = TCC_EVENT_GENERATION_SELECTION_START;
	tcc_event_config.output_config.modify_generation_selection = true;
	//tcc_event_config.generate_event_on_channel[TCC_NUM_CHANNELS-TCC_NUM_CHANNELS] = true;
	tcc_event_config.generate_event_on_counter_event = true;	
	tcc_enable_events(&tcc_instance, &tcc_event_config);
	tcc_enable(&tcc_instance);
}
