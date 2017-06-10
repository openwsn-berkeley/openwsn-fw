/**
* Copyright (c) 2014 Atmel Corporation. All rights reserved. 
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* 
* 3. The name of Atmel may not be used to endorse or promote products derived 
* from this software without specific prior written permission.  
* 
* 4. This software may only be redistributed and used in connection with an 
* Atmel microcontroller product.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* 
*/

/* === INCLUDES ============================================================ */
#include "samr21_timer.h"
#include "sctimer.h"
#include "board_info.h"
#include "leds.h"
#include "debugpins.h"
#include "gpio.h"

/* === MACROS ============================================================== */
#define RADIO_TIMER_PERIOD UINT16_MAX

#define TIMERLOOP_THRESHOLD          0x4000     // 0.5 seconds @ 32768Hz clock
#define MINIMUM_COMPAREVALE_ADVANCE  10
/* === PROTOTYPE ============================================================== */

/* === GLOBALS ============================================================= */

typedef struct {
    sctimer_cbt sctimer_cb;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars;

/*
 * @brief radiotimer_init  Initialize the radio timer 
 *        with 32.768KHz input clock 
 *
 * @param None
 *
 */
void sctimer_init(void)
{
    memset(&sctimer_vars,0,sizeof(sctimer_vars_t));
    timer4_init();
    tc4_irq_enable();
    tc4_disable_overflow_isr();
    tc4_disable_compare1_isr();
    tc4_set_count(0);
}

/*
 * @brief sctimer_set_callback  Register the sc timer 
 *        compare callback 
 *
 * @param cb callback address pointer
 *
 */
void sctimer_set_callback(sctimer_cbt cb)
{
 sctimer_vars.sctimer_cb     = cb;
}

/*
 * @brief sctimer_readCounter   Get the current timer 
 *        counter value
 *
 * @param return PORT_RADIOTIMER_WIDTH counter value
 *
 */
PORT_RADIOTIMER_WIDTH sctimer_readCounter(void)
{
    PORT_RADIOTIMER_WIDTH time_val;
    dbg_pin1_set();    
    time_val = (PORT_RADIOTIMER_WIDTH)tc4_get_count();
    dbg_pin1_clr();    
    return (time_val);
}

void sctimer_setCompare(PORT_TIMER_WIDTH val){
    tc4_enable_compare1_isr(); 
    if (tc4_get_count() - val < TIMERLOOP_THRESHOLD){
        // the timer is already late, schedule the ISR right now manually 
        // to do
    } else {
        if (val-tc4_get_count()<MINIMUM_COMPAREVALE_ADVANCE){
            // there is hardware limitation to schedule the timer within TIMERTHRESHOLD ticks
            // schedule ISR right now manually
            // to do
        } else {
            // schedule the timer at val
            tc4_set_compare1((val));
        }
    }
}

/*
 * @brief sctimer_disable cancel the running radio timer
 *        But this will not stop the timer 
 *
 * @param None
 *
 */
void sctimer_disable(void)
{  
  tc4_disable_compare1_isr();
}

/*
 * @brief sctimer_enable enable the running radio timer
 *
 * @param None
 *
 */
void sctimer_enable(void){
    tc4_enable_compare1_isr(); 
}


/*
 * @brief tc4_cca1_callback  Timer module compare call back 
 *
 * @param module_instance stored timer module instance
 *
 */
void tc4_cca1_callback(void)
{
  debugpins_isr_set();
 if (sctimer_vars.sctimer_cb != NULL) 
 {
     // clear flag?
     sctimer_vars.sctimer_cb();
 }    
 debugpins_isr_clr();
}
