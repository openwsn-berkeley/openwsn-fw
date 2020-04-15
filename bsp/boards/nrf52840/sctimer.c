/**
 * brief A timer module with only a single compare value. 
 *
 * Authors: Tamas Harczos (1, tamas.harczos@imms.de) and Adam Sedmak (2, adam.sedmak@gmail.com)
 * Company: (1) Institut fuer Mikroelektronik- und Mechatronik-Systeme gemeinnuetzige GmbH (IMMS GmbH)
 *          (2) Faculty of Electronics and Computing, Zagreb, Croatia
 *    Date: May 2018
 *
 *    Note: We use RTC0 peripheral with its CC0 register.
*/


#if defined(UNIT_TESTING)
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf52840_peripherals.h"
#endif
#include "sdk/components/boards/boards.h"

#include "nrfx_rtc_hack.h" ///< the implementation is based on the hacked version of Nordic's Real-Time Counter (RTC) driver, which allows us to schedule an RTC0 interrupt with CC1 event, triggered "by hand"
#include "sdk/components/libraries/delay/nrf_delay.h"

#include "sctimer.h"
#include "board.h"
#include "leds.h"
#include "debugpins.h"


// ========================== define ==========================================

#define MINIMUM_ISR_ADVANCE         16         ///< number of ticks to set CC ahead to make sure the RTC will fire (should this be equal to TIMERTHRESHOLD of opentimers?)
#define TIMERLOOP_THRESHOLD         0x20000   ///< 3s, if sctimer_setCompare() is late by max that many ticks, we still issue the ISR 
#define MAX_RTC_TASKS_DELAY         47        ///< maximum delay in us until an RTC config task is executed


// ========================== variable ========================================

typedef struct
{
  sctimer_cbt         cb;
  uint32_t            last_counter;
  uint32_t            counter_MSB;      ///< the first 8 bits of the 32 bit counter (which do not exist in the physical timer)
  uint32_t            cc32bit_MSB;      ///< the first 8 bits of the 32 bit CC (capture and compare) value, set
  bool                RTC_enabled;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars= {0};

nrfx_rtc_t m_timer= NRFX_RTC_INSTANCE(0);



// ========================== prototypes========================================

extern bool ieee154e_isSynch(void);

void timer_event_handler(nrfx_rtc_int_type_t int_type);


// ========================== protocol =========================================

/**
\brief Initialization sctimer.
*/
void sctimer_init(void) {
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
    if (NRFX_SUCCESS != retVal) {
        leds_error_blink();
        board_reset();
    }
    nrf_delay_us(MAX_RTC_TASKS_DELAY);

    // disable interrupt and event for overflow
    nrfx_rtc_overflow_disable(&m_timer);

    // reset counter
    nrfx_rtc_counter_clear(&m_timer);
    nrf_delay_us(MAX_RTC_TASKS_DELAY);

    // from this on, the RTC will run, but also draw electrical current
    sctimer_enable();
}

void sctimer_set_callback(sctimer_cbt cb) {
    sctimer_vars.cb= cb;
}

/**
\brief set compare interrupt
*/
void sctimer_setCompare(PORT_TIMER_WIDTH val) {

    uint32_t counter_current= sctimer_readCounter();

    if (counter_current - val < TIMERLOOP_THRESHOLD) {
        // the timer is already late, schedule the ISR right now manually 
        setIntPending_RTC0_CC1();
    } else {
        if (val - counter_current < MINIMUM_ISR_ADVANCE)  {
            // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
            // schedule ISR right now manually
            setIntPending_RTC0_CC1();
        } else {
            // schedule the timer at val
            nrfx_rtc_cc_set(&m_timer, 0, val & 0x00FFFFFF, true);         ///< set 3 LSBs of CC
        }
    }
}

/**
\brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH sctimer_readCounter(void) {
   uint32_t current_counter= nrfx_rtc_counter_get(&m_timer);

    NRFX_CRITICAL_SECTION_ENTER();

    if (current_counter < sctimer_vars.last_counter) {
        // 24-bit overflow happened
        sctimer_vars.counter_MSB += 0x01000000;
    }

    sctimer_vars.last_counter= current_counter;
    current_counter |= sctimer_vars.counter_MSB;

    NRFX_CRITICAL_SECTION_EXIT();

    return current_counter;
}

void sctimer_enable(void) {

    if (!sctimer_vars.RTC_enabled) {
        // power on RTC instance
        sctimer_vars.RTC_enabled= true;
        nrfx_rtc_enable(&m_timer);
        nrf_delay_us(MAX_RTC_TASKS_DELAY);
    }
}

void sctimer_disable(void) {
    if (sctimer_vars.RTC_enabled) {
        // power down RTC instance
        sctimer_vars.RTC_enabled= false;
        nrfx_rtc_disable(&m_timer);
        nrf_delay_us(MAX_RTC_TASKS_DELAY);
    }
}


//=========================== interrupt handler ===============================

void timer_event_handler(nrfx_rtc_int_type_t int_type) {
    debugpins_isr_set();

    if (
        (sctimer_vars.cb != 0)  &&
        ( 
            (int_type == NRFX_RTC_INT_COMPARE1) || 
            (int_type == NRFX_RTC_INT_COMPARE0) 
        )
    ) {
        sctimer_vars.cb();  
    }

    debugpins_isr_clr();
}
