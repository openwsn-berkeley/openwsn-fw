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

#include <sam.h>
#include "samr21_gclk.h"



/** \brief gclk_chan_config Configure the GCLK for the given channel.

     This function will disables the GCLK before configuring for selected GCLK

    \param [in] channel
	\param [in] gclk_id 
    \return None
 */
void gclk_chan_config(const uint8_t channel, uint8_t gclk_id)
{
 /* Configure the generic clock for the module and enable it */
 
 /* Select the requested generator channel */
 *((uint8_t*)&GCLK->CLKCTRL.reg) = channel;
 
 /* Switch to known-working source so that the channel can be disabled */
 uint32_t prev_gen_id = GCLK->CLKCTRL.bit.GEN;
 GCLK->CLKCTRL.bit.GEN = 0;

 /* Disable the generic clock */
 GCLK->CLKCTRL.reg &= ~GCLK_CLKCTRL_CLKEN;
 while (GCLK->CLKCTRL.reg & GCLK_CLKCTRL_CLKEN) {
	 /* Wait for clock to become disabled */
 }

 /* Restore previous configured clock generator */
 GCLK->CLKCTRL.bit.GEN = prev_gen_id;
 
 /* Write the new configuration */
 GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(channel) | GCLK_CLKCTRL_GEN(gclk_id);
}

/** \brief gclk_enable Enable the GCLK for particular channel

     This function enable the GCLK for particular channel

    \param [in] channel 
    \return None
 */
void gclk_enable(const uint8_t channel)
{
    /* Enables a Generic Clock that was previously configured */
	/* Select the requested generator channel */
	*((uint8_t*)&GCLK->CLKCTRL.reg) = channel;

	/* Enable the clock anyway, since when needed it will be requested
	 * by External Interrupt driver */
	GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_CLKEN;
}

