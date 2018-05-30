/**
 * brief A timer module with only a single compare value. 
 *
 * Authors: Tamas Harczos (1, tamas.harczos@imms.de) and Adam Sedmak (2, adam.sedmak@gmail.com)
 * Company: (1) Institut für Mikroelektronik- und Mechatronik-Systeme gemeinnützige GmbH (IMMS GmbH)
 *          (2) Faculty of Electronics and Computing, Zagreb, Croatia
 * Date:   May 2018
*/

#include "sdk/modules/nrfx/drivers/include/nrfx_rtc.h"  ///< the implementation is based on Nordic's Real-Time Counter (RTC)
#include "sdk/components/libraries/delay/nrf_delay.h"

#include "sdk/components/boards/pca10056.h"
#include "sdk/components/boards/boards.h"

#include "sctimer.h"
#include "board.h"
#include "leds.h"
#include "debugpins.h"


// ========================== define ==========================================

#define MINIMUM_COMPAREVALE_ADVANCE 5
#define COMPAREVALE_ADVANCE_STEP 2
#define MAX_RTC_TASKS_DELAY 47          // maximum delay in us until an RTC config task is executed
#define CC_TIMER 0                      // CC channel used for the timer's compare value


// ========================== variable ========================================

typedef struct
{
  sctimer_cbt         cb;
  sctimer_capture_cbt startFrameCb;
  sctimer_capture_cbt endFrameCb;
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
    .reliable= 1 // NRFX_RTC_DEFAULT_CONFIG_RELIABLE
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

void sctimer_setStartFrameCb(sctimer_capture_cbt cb)
{
  sctimer_vars.startFrameCb= cb;
}

void sctimer_setEndFrameCb(sctimer_capture_cbt cb)
{
  sctimer_vars.endFrameCb= cb;
}

/**
\brief set compare interrupt
*/
void sctimer_setCompare(PORT_TIMER_WIDTH val)
{
  nrfx_err_t retVal= NRFX_SUCCESS;

  // make sure that no other CC match event will interrupt this block
  uint32_t int_mask= RTC_CHANNEL_INT_MASK(CC_TIMER);
  nrf_rtc_event_t event= RTC_CHANNEL_EVENT_ADDR(CC_TIMER);
  nrf_rtc_event_disable(m_timer.p_reg, int_mask);
  nrf_rtc_int_disable(m_timer.p_reg, int_mask);  

  uint32_t counter_current;
  uint32_t counter_distance, counter_demanded;

  uint8_t restart_count= 0;
  uint8_t const max_restart_count= 3;

restart:

  counter_current= sctimer_readCounter();

  if (val >= counter_current)
  {
    counter_distance= val - counter_current;
    if (counter_distance >= MINIMUM_COMPAREVALE_ADVANCE)
    {
      counter_demanded= val;
    }
    else
    {
      counter_demanded= (counter_current + MINIMUM_COMPAREVALE_ADVANCE) & 0xFFFFFFFF;
    }
  }
  else
  {
    if (0xFFFFFFFF-counter_current+1+val >= MINIMUM_COMPAREVALE_ADVANCE)
    {
      counter_demanded= val;
    }
    else
    {
      counter_demanded= (counter_current + MINIMUM_COMPAREVALE_ADVANCE) & 0xFFFFFFFF;
    }
  }

  // set MSB of CC
  sctimer_vars.cc32bit_MSB= counter_demanded & 0xFF000000;

  // set 3 LSBs of CC
  retVal= nrfx_rtc_cc_set(&m_timer, CC_TIMER, counter_demanded & 0x00FFFFFF, false);
  // nrf_delay_us(MAX_RTC_TASKS_DELAY);

  // nrfx_rtc_cc_set() will return NRFX_ERROR_TIMEOUT if the demanded CC value cannot be safely made by the timer.
  // In that case, we reschedule the next event.
  if ((NRFX_ERROR_TIMEOUT == retVal) && (restart_count < max_restart_count))
  {
    val += COMPAREVALE_ADVANCE_STEP;
    restart_count++;
    goto restart;
  }

  // re-enable CC event and interrupt
  nrf_rtc_event_clear(m_timer.p_reg,event);
  nrf_rtc_int_enable(m_timer.p_reg, int_mask);
  nrf_rtc_event_enable(m_timer.p_reg,int_mask);

  if (retVal != NRFX_SUCCESS)
  {
    leds_error_blink();
    board_reset();
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
}
