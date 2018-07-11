/**
 * brief A timer module with only a single compare value. 
 *
 * Authors: Tamas Harczos (1, tamas.harczos@imms.de) and Adam Sedmak (2, adam.sedmak@gmail.com)
 * Company: (1) Institut fuer Mikroelektronik- und Mechatronik-Systeme gemeinnuetzige GmbH (IMMS GmbH)
 *          (2) Faculty of Electronics and Computing, Zagreb, Croatia
 *    Date: May 2018
 *
 *    Note: We use RTC0 hardware with its CC0 register.
*/

#include "nrfx_rtc_hack.h"                    ///< the implementation is based on the hacked version of Nordic's Real-Time Counter (RTC) driver
#include "sdk/components/libraries/delay/nrf_delay.h"

#include "sdk/components/boards/pca10056.h"
#include "sdk/components/boards/boards.h"

#include "sctimer.h"
#include "board.h"
#include "leds.h"
#include "debugpins.h"

#if (ENABLE_SEGGER_SYSVIEW == 1)
#include "SEGGER_SYSVIEW.h"
#endif // (ENABLE_SEGGER_SYSVIEW == 1)


// ========================== define ==========================================

#define MINIMUM_COMPAREVALE_ADVANCE 10        ///< equal to TIMERTHRESHOLD of opentimers
#define TIMERLOOP_THRESHOLD         0x20000   ///< 3s, if sctimer_setCompare() is late by max that many ticks, we still issue the ISR 
#define MAX_RTC_TASKS_DELAY         47        ///< maximum delay in us until an RTC config task is executed


// ========================== variable ========================================

typedef struct
{
  sctimer_cbt         cb;
  uint8_t             f_SFDreceived;
  uint32_t            counter_MSB;      ///< the first 8 bits of the 32 bit counter (which do not exist in the physical timer)
  uint32_t            cc32bit_MSB;      ///< the first 8 bits of the 32 bit CC (capture and compare) value, set
} sctimer_vars_t;

sctimer_vars_t sctimer_vars= {0};

static nrfx_rtc_t m_timer= NRFX_RTC_INSTANCE(0);



// ========================== prototypes========================================

static void timer_event_handler(nrfx_rtc_int_type_t int_type);


// ========================== protocol =========================================

/**
\brief Initialization sctimer.
*/
void sctimer_init(void)
{
  nrfx_err_t retVal= NRFX_SUCCESS;
  memset(&sctimer_vars, 0, sizeof(sctimer_vars_t));

  nrfx_rtc_config_t const rtc_cfg=
  {
    .prescaler= RTC_FREQ_TO_PRESCALER(NRFX_RTC_DEFAULT_CONFIG_FREQUENCY),
    .interrupt_priority= NRFX_RTC_DEFAULT_CONFIG_IRQ_PRIORITY,
    .tick_latency= NRFX_RTC_US_TO_TICKS(NRFX_RTC_MAXIMUM_LATENCY_US, NRFX_RTC_DEFAULT_CONFIG_FREQUENCY),
    .reliable= 0
  };

  // initialize RTC, we use the 32768 Hz clock without prescaler
  retVal= nrfx_rtc_init(&m_timer, &rtc_cfg, timer_event_handler);
  if (NRFX_SUCCESS != retVal)
  {
    leds_error_blink();
    board_reset();
  }
  nrf_delay_us(MAX_RTC_TASKS_DELAY);

  // enable interrupts for overflow event
  nrfx_rtc_overflow_enable(&m_timer, true);

  // reset counter
  nrfx_rtc_counter_clear(&m_timer);
  nrf_delay_us(MAX_RTC_TASKS_DELAY);

  // from this on, the RTC will run, but also draw electrical current
  sctimer_enable();
}

void sctimer_set_callback(sctimer_cbt cb)
{
  sctimer_vars.cb= cb;
}

/**
\brief set compare interrupt
*/
void sctimer_setCompare(PORT_TIMER_WIDTH val)
{
  uint32_t counter_current;
  uint32_t distance_straight, distance_wrapped;

  counter_current= sctimer_readCounter();
#if (ENABLE_SEGGER_SYSVIEW == 1)
	uint32_t cc_current= NRF_RTC0->CC[0];
#endif

  if (val >= counter_current)
  {
    distance_straight= val - counter_current;
    distance_wrapped= (0xFFFFFFFF-val+1) + counter_current;

    if ( (distance_wrapped < TIMERLOOP_THRESHOLD) ||                ///< val is higher, but actually counter is ahead and already in the next wrap
         (distance_straight < MINIMUM_COMPAREVALE_ADVANCE) )        ///< val is ahead, but the difference is not enough for the timer to make it in time
    {
#if (ENABLE_SEGGER_SYSVIEW == 1)
      SEGGER_SYSVIEW_PrintfHost("setIntPending_RTC0_CC0()");
#endif // (ENABLE_SEGGER_SYSVIEW == 1)
      setIntPending_RTC0_CC0();
    }

    else                                                            ///< other cases are not problematic
    {
#if (ENABLE_SEGGER_SYSVIEW == 1)
      SEGGER_SYSVIEW_PrintfHost("Counter=%u, CC_old=%u, CC_new=%u", counter_current, cc_current, val);
#endif // (ENABLE_SEGGER_SYSVIEW == 1)
      sctimer_vars.cc32bit_MSB= val & 0xFF000000;                   ///< set MSB of CC 
      nrfx_rtc_cc_set(&m_timer, 0, val & 0x00FFFFFF, true);         ///< set 3 LSBs of CC
    }
  }

  else
  {
    distance_straight= counter_current - val;
    distance_wrapped= (0xFFFFFFFF-counter_current+1) + val;

    if ( (distance_wrapped < MINIMUM_COMPAREVALE_ADVANCE) ||  ///< val is ahead, but the difference is not enough for the timer to make it in time
         (distance_straight < TIMERLOOP_THRESHOLD) )          ///< val is lagging, but within TIMERLOOP limits
    {
#if (ENABLE_SEGGER_SYSVIEW == 1)
      SEGGER_SYSVIEW_PrintfHost("setIntPending_RTC0_CC0()");
#endif // (ENABLE_SEGGER_SYSVIEW == 1)
      setIntPending_RTC0_CC0();
    }

    else                                                      ///< other cases are not problematic
    {
#if (ENABLE_SEGGER_SYSVIEW == 1)
      SEGGER_SYSVIEW_PrintfHost("Counter=%u, CC_old=%u, CC_new=%u", counter_current, cc_current, val);
#endif // (ENABLE_SEGGER_SYSVIEW == 1)
      sctimer_vars.cc32bit_MSB= val & 0xFF000000;                   ///< set MSB of CC 
      nrfx_rtc_cc_set(&m_timer, 0, val & 0x00FFFFFF, true);         ///< set 3 LSBs of CC
    }
  }
}

/**
\brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH sctimer_readCounter(void)
{
  return( sctimer_vars.counter_MSB | nrfx_rtc_counter_get(&m_timer) );
}

void sctimer_enable(void)
{
  // power on RTC instance
  nrfx_rtc_enable(&m_timer);
  nrf_delay_us(MAX_RTC_TASKS_DELAY);
}

void sctimer_disable(void)
{
  // power down RTC instance
  nrfx_rtc_disable(&m_timer);
  nrf_delay_us(MAX_RTC_TASKS_DELAY);
}


//=========================== interrupt handler ===============================

static void timer_event_handler(nrfx_rtc_int_type_t int_type)
{
#if (ENABLE_SEGGER_SYSVIEW == 1)
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  debugpins_isr_set();

  if (int_type == NRFX_RTC_INT_OVERFLOW)
  {
    sctimer_vars.counter_MSB += 0x01000000;
  }

  // if we reached the specified CC value within the current wrap AND it is the right wrap, call callback
  else if ((int_type == NRFX_RTC_INT_COMPARE0) && (sctimer_vars.cc32bit_MSB == sctimer_vars.counter_MSB) && (sctimer_vars.cb != 0))
  {
    sctimer_vars.cb();  
  }

  debugpins_isr_clr();
#if (ENABLE_SEGGER_SYSVIEW == 1)
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}
