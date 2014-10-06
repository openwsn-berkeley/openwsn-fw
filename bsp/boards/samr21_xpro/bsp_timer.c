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

#include "bsp_timer.h"
#include "debugpins.h"
#include "samr21_timer.h"
#include "board_info.h"

/* === MACROS ============================================================== */

#define TIMER_PERIOD UINT16_MAX
/* === Typedef ============================================================= */
/* Structure to hold the bsp timer related functionalities */
typedef struct
{
	bsp_timer_cbt cb;
	PORT_TIMER_WIDTH last_compare_value;
} bsp_timer_vars_t;

/* This variables keeps information about bsp timer and callbacks */
bsp_timer_vars_t bsp_timer_vars;

/* === GLOBALS ============================================================= */

/* === PROTOTYPE ============================================================= */
/* Call back handler for capture compare */

/* 
 * @brief bsp_timer_init  will Initialize the BSP Timer with default configuration
 *
 * @param None
 */
void bsp_timer_init(void)
{
 timer3_init();
}

/* 
 * @brief bsp_timer_set_callback  Register the call back for bsp timer
 *
 * @param cb function pointer to the bsp_timer call back function
 */
void bsp_timer_set_callback(bsp_timer_cbt cb)
{
	bsp_timer_vars.cb = cb;
	tc3_irq_enable();
}

/* 
 * @brief tc3_cca0_callback  call back function for bsp_timer called from ISR
 *
 * @param module_instance - timer module instance from interrupt
 */
void tc3_cca0_callback(void)
{
	/* Capture Compare 0 callback */
	debugpins_isr_set();
	if(bsp_timer_vars.cb != NULL)
	{
		bsp_timer_vars.cb();
	}	
	debugpins_isr_clr();
}

/* 
 * @brief bsp_timer_reset  reset the bsp timer to default but do not stop the timer
 *
 * @param None
 */
void bsp_timer_reset(void)
{
	//disable will clears the compare interrupt
	tc3_disable_compare0_isr();	
	tc3_set_compare0(TIMER_PERIOD);
	// reset timer -- set counter to 0
	tc3_set_count(0);
	// record last timer compare value
	bsp_timer_vars.last_compare_value =  0;
}

/**
\brief Schedule the callback to be called in some specified time.

The delay is expressed relative to the last compare event. It doesn't matter
how long it took to call this function after the last compare, the timer will
expire precisely delayTicks after the last one.

The only possible problem is that it took so long to call this function that
the delay specified is shorter than the time already elapsed since the last
compare. In that case, this function triggers the interrupt to fire right away.

This means that the interrupt may fire a bit off, but this inaccuracy does not
propagate to subsequent timers.

\param delayTicks Number of ticks before the timer expired, relative to the
                  last compare event.
*/
void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks)
{
   PORT_TIMER_WIDTH newCompareValue;
   PORT_TIMER_WIDTH temp_last_compare_value;
   PORT_TIMER_WIDTH current_value;
   
   temp_last_compare_value = bsp_timer_vars.last_compare_value;   
   newCompareValue = bsp_timer_vars.last_compare_value+delayTicks+1;
   bsp_timer_vars.last_compare_value = newCompareValue;
   current_value = (PORT_TIMER_WIDTH)tc3_get_count();
   
   if (delayTicks < (current_value-temp_last_compare_value))
   {
	   /* This needs to be fixed */
	   tc3_enable_compare0_isr();
   }
   else
   { 	   
       /* this is the normal case, have timer expire at newCompareValue */
	   tc3_disable_compare0_isr();
	   tc3_set_compare0(newCompareValue);
	   tc3_enable_compare0_isr();
   }	
}

/* 
 * @brief bsp_timer_cancel_schedule  Cancel the running compare
 *
 * @param None
 */
void bsp_timer_cancel_schedule(void)
{
	tc3_disable_compare0_isr();
	tc3_set_compare0(TIMER_PERIOD);
}

/* 
 * @brief bsp_timer_get_currentValue  Get current timer counter value
 *
 * @param PORT_TIMER_WIDTH return current bsp timer counter value
 */
PORT_TIMER_WIDTH bsp_timer_get_currentValue(void)
{
	return ((PORT_TIMER_WIDTH)tc3_get_count());
}
