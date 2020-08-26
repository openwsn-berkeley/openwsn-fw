/**
\brief A timer module with only a single compare value.

\author: Tengfei Chang <tengfei.chang@inria.fr> August 2020
*/

#include "nRF5340_network.h"
#include "debugpins.h"
#include "sctimer.h"
#include "board.h"
#include "leds.h"

// ========================== define ==========================================

#define TIMERLOOP_THRESHOLD             0xffff
#define MINIMUM_COMPAREVALE_ADVANCE     5  // nRF52840_PS_v1.0: 2 (page 337)

// LFCLK
#define LFCLK_SRC_RC          0x0 // RC oscillator 
#define LFCLK_SRC_XTAL        0x1 // crystal oscillator
#define LFCLK_SRC_SYNTH       0x2 // synthesized from HFCLK
#define LFCLK_STATE_RUNNING_MASK   ((uint32_t)(0x01))<<16

// RTC
#define RTC_TICK_SHIFT        0
#define RTC_OVRFLW_SHIFT      1
#define RTC_COMPARE0_SHIFT    16
#define RTC_COMPARE1_SHIFT    17
#define RTC_COMPARE2_SHIFT    18
#define RTC_COMPARE3_SHIFT    19

// ========================== variable ========================================

typedef struct {
    sctimer_cbt         sctimer_cb;
    uint8_t             f_SFDreceived;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars;

// ========================== private ==========================================

// ========================== protocol =========================================

void sctimer_init(void){

    memset(&sctimer_vars, 0, sizeof(sctimer_vars_t));

    // stop LFCLK
    NRF_CLOCK_NS->TASKS_LFCLKSTOP     = 1;
    while (
      (NRF_CLOCK_NS->LFCLKSTAT & LFCLK_STATE_RUNNING_MASK)
    );


    // configure the source
    NRF_CLOCK_NS->LFCLKSRC = (uint32_t)LFCLK_SRC_XTAL;

    // start LFCLK
    NRF_CLOCK_NS->TASKS_LFCLKSTART = (uint32_t)1;
    while(
      (NRF_CLOCK_NS->LFCLKSTAT & LFCLK_STATE_RUNNING_MASK) == 0
    );
    
    // stop RTC timer
    NRF_RTC0_NS->TASKS_STOP = (uint32_t)1;

    // configure compare timer interrupt
    NRF_RTC0_NS->EVTEN = 
          ((uint32_t)0)<<RTC_TICK_SHIFT
        | ((uint32_t)0)<<RTC_OVRFLW_SHIFT
        | ((uint32_t)1)<<RTC_COMPARE0_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE1_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE2_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE3_SHIFT;

    // set priority and enable interrupt in NVIC
    NVIC->IPR[((uint32_t)RTC0_IRQn)] = 
        (uint8_t)(
            (
                RTC_PRIORITY << (8 - __NVIC_PRIO_BITS)
            ) & (uint32_t)0xff
        );
    NVIC->ISER[((uint32_t)RTC0_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RTC0_IRQn) & 0x1f);

    // set prescale
    NRF_RTC0_NS->PRESCALER = (uint32_t)0;

    // start RTC timer
    NRF_RTC0_NS->TASKS_CLEAR = (uint32_t)1;
    NRF_RTC0_NS->TASKS_START = (uint32_t)1;
}

void sctimer_set_callback(sctimer_cbt cb){
    sctimer_vars.sctimer_cb = cb;
}

void sctimer_setCompare(PORT_TIMER_WIDTH val){
    
    // enable interrupt
     NRF_RTC0_NS->INTENSET = 
          ((uint32_t)0)<<RTC_TICK_SHIFT
        | ((uint32_t)0)<<RTC_OVRFLW_SHIFT
        | ((uint32_t)1)<<RTC_COMPARE0_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE1_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE2_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE3_SHIFT;

    if ( NRF_RTC0_NS->COUNTER- val < TIMERLOOP_THRESHOLD){
        // the timer is already late, schedule the ISR right now manually
        NRF_RTC0_NS->EVENTS_COMPARE[0] = (uint32_t)1;
    } else {
        if (val-NRF_RTC0_NS->COUNTER<MINIMUM_COMPAREVALE_ADVANCE){
            // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
            // schedule ISR right now manually
            NRF_RTC0_NS->EVENTS_COMPARE[0] = (uint32_t)1;
        } else {
            // schedule the timer at val
            NRF_RTC0_NS->CC[0] = (uint32_t)val;
        }
    }
}

PORT_TIMER_WIDTH sctimer_readCounter(void){
    
    return NRF_RTC0_NS->COUNTER;
}

void sctimer_enable(void){

    // enable interrupt
     NRF_RTC0_NS->INTENSET = 
          ((uint32_t)0)<<RTC_TICK_SHIFT
        | ((uint32_t)0)<<RTC_OVRFLW_SHIFT
        | ((uint32_t)1)<<RTC_COMPARE0_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE1_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE2_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE3_SHIFT;
}

void sctimer_disable(void){

    // disable interrupt
     NRF_RTC0_NS->INTENSET = 
          ((uint32_t)0)<<RTC_TICK_SHIFT
        | ((uint32_t)0)<<RTC_OVRFLW_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE0_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE1_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE2_SHIFT
        | ((uint32_t)0)<<RTC_COMPARE3_SHIFT;
}

// ========================== private =========================================


void RTC0_IRQHandler(void){
    if (NRF_RTC0_NS->EVENTS_COMPARE[0]){
        sctimer_isr();
    }
}

//=========================== interrupt handlers ==============================

kick_scheduler_t sctimer_isr(void) {
    debugpins_isr_set();
    if (sctimer_vars.sctimer_cb!=NULL) {
        NRF_RTC0_NS->EVENTS_COMPARE[0] = (uint32_t)0;
        sctimer_vars.sctimer_cb();
        debugpins_isr_clr();
        return KICK_SCHEDULER;
    }
    debugpins_isr_clr();
    return DO_NOT_KICK_SCHEDULER;
}
