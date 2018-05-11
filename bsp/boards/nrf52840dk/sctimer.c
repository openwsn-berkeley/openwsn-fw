/**
 * brief A timer module with only a single compare value. 
 *
 * Author: Adam Sedmak (adam.sedmak@gmail.com)
 * Company: Faculty of Electronics and Computing, Zagreb, Croatia
 * Date:   Apr 2018
*/

#include "sdk/components/boards/pca10056.h"
#include "sdk/components/boards/boards.h"
#include "sdk/modules/nrfx/drivers/include/nrfx_timer.h"

#include "sctimer.h"
#include "board.h"
#include "leds.h"

// ========================== define ==========================================

#define TIMERLOOP_THRESHOLD          0x4000     // 0.5 seconds @ 32768Hz clock
#define MINIMUM_COMPAREVALE_ADVANCE  3
#define SCTIMER_INSTANCE             0

// ========================== variable ========================================

typedef struct {
    sctimer_cbt         cb;
    sctimer_capture_cbt startFrameCb;
    sctimer_capture_cbt endFrameCb;
    uint8_t             f_SFDreceived;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars;

const nrfx_timer_t timer = NRFX_TIMER_INSTANCE(SCTIMER_INSTANCE);

// ========================== prototypes========================================

void timer_event_handler(nrf_timer_event_t const * p_event, void * p_context);

// ========================== protocol =========================================

/**
\brief Initialization sctimer.
*/
void sctimer_init(void)
{
    memset(&sctimer_vars, 0, sizeof(sctimer_vars_t));
    nrfx_timer_config_t nrfx_timer = NRFX_TIMER_DEFAULT_CONFIG;

    if(NRFX_SUCCESS != nrfx_timer_init(&timer, &nrfx_timer, timer_event_handler))
    {
        leds_error_blink();
        board_reset();
    }
}

void sctimer_set_callback(sctimer_cbt cb)
{
    sctimer_vars.cb = cb;
}

void sctimer_setStartFrameCb(sctimer_capture_cbt cb)
{
    sctimer_vars.startFrameCb = cb;
}

void sctimer_setEndFrameCb(sctimer_capture_cbt cb)
{
    sctimer_vars.endFrameCb = cb;
}

/**
\brief set compare interrupt
*/
void sctimer_setCompare(PORT_TIMER_WIDTH val)
{

}

/**
\brief Return the current value of the timer's counter.

 \returns The current value of the timer's counter.
*/
PORT_TIMER_WIDTH sctimer_readCounter(void)
{

}

void sctimer_enable(void)
{
  nrfx_timer_enable(&timer);
}

void sctimer_disable(void)
{
  nrfx_timer_disable(&timer);
}

// ========================== private =========================================

void timer_event_handler(nrf_timer_event_t const * p_event,              
                         void *                    p_context)
{

}

//=========================== interrupt handlers ==============================

/**
\brief TimerB CCR1-6 interrupt service routine
*/
kick_scheduler_t sctimer_isr(void) {
   PORT_TIMER_WIDTH tbiv_local;
   
   // reading TBIV returns the value of the highest pending interrupt flag
   // and automatically resets that flag. We therefore copy its value to the
   // tbiv_local local variable exactly once. If there is more than one 
   // interrupt pending, we will reenter this function after having just left
   // it.
   
   return DO_NOT_KICK_SCHEDULER;
}
