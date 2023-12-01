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

#include "nrf52840.h"
#include "sctimer.h"
#include "debugpins.h"

// ========================== define ==========================================

#define LFCLKSRC_SRC_POS      0
#define LFCLKSRC_BYPASS_POS   16
#define LFCLKSRC_EXTERNAL_POS 17

#define LFCLKSTAT_SRC_POS     0
#define LFCLKSTAT_STATE_POS   16

#define MINIMUM_ISR_ADVANCE         5        // nRF52840_PS_v1.7 (page 370)
#define TIMERLOOP_THRESHOLD         0xffff 
#define TIMERMASK                   0xffffff

// ========================== variable ========================================

typedef struct {
    sctimer_cbt         cb;
    uint8_t             lastFireMode;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars= {0};

// ========================== prototypes========================================

// ========================== protocol =========================================

/**
\brief Initialization sctimer.
*/
void sctimer_init(void) {

    memset(&sctimer_vars, 0, sizeof(sctimer_vars_t));

    NVIC->IP[RTC0_IRQn]         = (uint8_t)((RTC_PRIORITY << (8 - __NVIC_PRIO_BITS)) & (uint32_t)0xFF);
    NVIC->ISER[RTC0_IRQn>>5]    = (uint32_t)(0x1 << (RTC0_IRQn & 0x1f));

    // stop LFCLK
    NRF_CLOCK->TASKS_LFCLKSTOP     = 1;
    while((NRF_CLOCK->LFCLKSTAT & (1<<LFCLKSTAT_STATE_POS)) == 1);
    
    // configure prescaler
    NRF_RTC0->PRESCALER         = 0;

    // configure the source
    NRF_CLOCK->LFCLKSRC = (1<<LFCLKSRC_SRC_POS);

    // start LFCLK
    NRF_CLOCK->TASKS_LFCLKSTART = (uint32_t)1;
    while((NRF_CLOCK->LFCLKSTAT & (1<<LFCLKSTAT_STATE_POS)) == 0);


    // enable compare 0 1, and 2
    NRF_RTC0->EVENTS_COMPARE[0] = 0;
    NRF_RTC0->INTENSET          = 0x1<<16;

    NRF_RTC0->TASKS_START       = 1;

    NVIC->IP[SWI0_EGU0_IRQn]         = (uint8_t)((RTC_PRIORITY << (8 - __NVIC_PRIO_BITS)) & (uint32_t)0xFF);
    NVIC->ISER[SWI0_EGU0_IRQn>>5]    = (uint32_t)(0x1 << (SWI0_EGU0_IRQn & 0x1f));

    // configure egu for manually interrupts
    NRF_EGU0->EVENTS_TRIGGERED[0] = 0;
    NRF_EGU0->INTENSET            = 1;
}

void sctimer_set_callback(sctimer_cbt cb) {
    sctimer_vars.cb= cb;
}

/**
\brief set compare interrupt
*/
void sctimer_setCompare(PORT_TIMER_WIDTH val) {

    uint32_t counter_current;
    
    counter_current = NRF_RTC0->COUNTER;

    if (((counter_current - val) & TIMERMASK) < TIMERLOOP_THRESHOLD) {
        // the timer is already late, schedule the ISR right now manually 
        NRF_EGU0->TASKS_TRIGGER[0] = 1;
        sctimer_vars.lastFireMode  = 1;
    } else {
        if (((val - counter_current) & TIMERMASK) < MINIMUM_ISR_ADVANCE)  {
            // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
            // schedule ISR right now manually
            NRF_EGU0->TASKS_TRIGGER[0] = 1;
            sctimer_vars.lastFireMode  = 2;
        } else {
            // schedule the timer at val
            NRF_RTC0->CC[0] = val;
            sctimer_vars.lastFireMode = 0;
        }
    }
}

/**
\brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH sctimer_readCounter(void) {

    return NRF_RTC0->COUNTER;
}

void sctimer_enable(void) {

    NRF_RTC0->INTENSET = (uint32_t)(1<<16);
}

void sctimer_disable(void) {

    NRF_RTC0->INTENCLR = (uint32_t)(1<<16);
}


//=========================== interrupt handler ===============================

void RTC0_IRQHandler(void) {

    debugpins_isr_set();
    
    if (NRF_RTC0->EVENTS_COMPARE[0] != 0) {
        NRF_RTC0->EVENTS_COMPARE[0] = 0;

        if (sctimer_vars.cb != NULL) {
            sctimer_vars.cb();
        }
    }

    debugpins_isr_clr();
}


void SWI0_EGU0_IRQHandler(void) {

    debugpins_isr_set();
    
    if (NRF_EGU0->EVENTS_TRIGGERED[0] != 0) {
        NRF_EGU0->EVENTS_TRIGGERED[0] = 0;

        if (sctimer_vars.cb != NULL) {
            sctimer_vars.cb();
        }
    }

    debugpins_isr_clr();
}