/**
 * brief A timer module with only a single compare value. 
 *
 * Author: Adam Sedmak (1, adam.sedmak@gmail.com) and Tamas Harczos (2, tamas.harczos@imms.de)
 * Company: (1) Faculty of Electronics and Computing, Zagreb, Croatia
 *          (2) Institut für Mikroelektronik- und Mechatronik-Systeme gemeinnützige GmbH (IMMS GmbH)
 * Date:   Apr 2018
*/

#include "sdk/components/libraries/timer/app_timer.h"  ///< the implementation is based on the Nordic's AppTimer, which uses RTC1 (Real-Time Counter)
#include "sdk/components/libraries/delay/nrf_delay.h"

#include "sdk/components/boards/pca10056.h"
#include "sdk/components/boards/boards.h"

#include "sctimer.h"
#include "board.h"
#include "leds.h"
#include "debugpins.h"
#include "uart.h"           ///< for debuggin only, REMOVE ME


// ========================== define ==========================================

#define MINIMUM_COMPAREVALE_ADVANCE  APP_TIMER_MIN_TIMEOUT_TICKS  // defaults to 5


// ========================== variable ========================================

typedef struct
{
  sctimer_cbt         cb;
  sctimer_capture_cbt startFrameCb;
  sctimer_capture_cbt endFrameCb;
  uint8_t             f_SFDreceived;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars;

APP_TIMER_DEF(m_timer);   ///< instantiate app_timer


// ========================== prototypes========================================

static void timer_event_handler(void *p_context);


// ========================== protocol =========================================

/**
\brief Initialization sctimer.
*/
void sctimer_init(void)
{
  memset(&sctimer_vars, 0, sizeof(sctimer_vars_t));

  // initialize app_timer, default configuration conveniently uses the 32kHz clock without prescaler
  app_timer_init();

    if (NRF_SUCCESS != app_timer_create(&m_timer, APP_TIMER_MODE_SINGLE_SHOT, timer_event_handler))
    {
      leds_error_blink();
      board_reset();
    }

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
   uint32_t retVal;

  // It seems sctimer_setCompare() explicitly wants to set the compare value, whereas app_timer_start()
  // is "intelligent enough" to require a timeout value, that's why the subtraction is needed.
  // Furthermore, if the timeout is too soon, we will need to round up to the shortes possible delay.
  uint32_t relTick= val-sctimer_readCounter();
  if (relTick < APP_TIMER_MIN_TIMEOUT_TICKS)
  {
    relTick= APP_TIMER_MIN_TIMEOUT_TICKS;
  }
  retVal= app_timer_start(m_timer, relTick, NULL);

  if (retVal != NRF_SUCCESS)
  {
    uart_writeByte(retVal);    ///< DEBUG, REMOVE ME!

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
  return app_timer_cnt_get();
}

void sctimer_enable(void)
{
  app_timer_resume();
}

void sctimer_disable(void)
{
  app_timer_pause();
}


//=========================== interrupt handler ===============================

static void timer_event_handler(void *p_context)
{
  debugpins_isr_set();
  
  // if a callback was specified, call it now
  if (sctimer_vars.cb != 0)
  {
    sctimer_vars.cb();  
  }

  debugpins_isr_clr();
}
