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
#include "radiotimer.h"
#include "board_info.h"
#include "leds.h"
#include "debugpins.h"
#include "gpio.h"

/* === MACROS ============================================================== */
#define RADIO_TIMER_PERIOD UINT16_MAX
/* === PROTOTYPE ============================================================== */

/* === GLOBALS ============================================================= */


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
    memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
    timer4_init();
	tc4_irq_enable();	
}

/*
 * @brief tc4_ovf_callback  Timer module overflow or 
 *        match frequency callback 
 *
 * @param module_instance stored timer module instance
 *
 */
void tc4_ovf_callback(void)
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
 * @brief tc4_cca1_callback  Timer module compare call back 
 *
 * @param module_instance stored timer module instance
 *
 */
void tc4_cca1_callback(void)
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
 tc4_disable_overflow_isr();
 tc4_disable_compare1_isr();
 tc4_set_count(0);
 tc4_set_compare0(period);
 tc4_enable_overflow_isr();
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
	time_val = (PORT_RADIOTIMER_WIDTH)tc4_get_count();
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
 tc4_disable_overflow_isr();
 tc4_set_compare0(period);
 tc4_enable_overflow_isr();
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
 timer_period_val = (PORT_RADIOTIMER_WIDTH)tc4_get_capture0_val();
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
  tc4_disable_compare1_isr();
  tc4_set_compare1((offset));
  tc4_enable_compare1_isr(); 
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
  tc4_disable_compare1_isr();
  tc4_set_compare1(RADIO_TIMER_PERIOD);
}

/*
 * @brief radiotimer_getCapturedTime Read the radio timer current count value
 *
 * @param return PORT_RADIOTIMER_WIDTH count value
 *
 */
inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(void)
{
  return ((PORT_RADIOTIMER_WIDTH)tc4_get_count());
}
