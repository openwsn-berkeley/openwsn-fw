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
#include "samr21_extint.h"
#include "samr21_gpio.h"

void ext_int0_callback(void) __attribute__ ((weak, alias("dummy_irq")));

void eic_irq_init(uint8_t pin, pinmux_t *pinmux, extint_t *extint)
{	
	pinmux_config(pin, pinmux);
			
	/* Get a pointer to the module hardware instance */
	Eic *const eic[EIC_INST_NUM] = EIC_INSTS;
	Eic *const eics = eic[extint->channel/32];
	 
	uint32_t config_pos = (4 * (extint->channel % 8));
	uint32_t new_config;

	/* Determine the channel's new edge detection configuration */
	new_config = (extint->detect << EIC_CONFIG_SENSE0_Pos) | EIC_CONFIG_FILTEN0;

	/* Clear the existing and set the new channel configuration */
	eics->CONFIG[extint->channel / 8].reg = (eics->CONFIG[extint->channel / 8].reg & \
	((EIC_CONFIG_SENSE0_Msk | EIC_CONFIG_FILTEN0) << config_pos)) | \
	(new_config << config_pos);

	/* Set the channel's new wake up mode setting */
	eics->WAKEUP.reg |=  (1UL << extint->channel);
	extint_enable_irq(extint->channel);
}

/** \brief at86rfx_extint_flag_clear Clears the edge detection state of a configured channel

     Clears the current state of a configured channel, readying it for the next level or edge detection.

    \param [in] None
    \return None
 */
void extint_flag_clear(uint8_t channel)
{
 /* Get a pointer to the module hardware instance */
 Eic *const eic[EIC_INST_NUM] = EIC_INSTS;
 Eic *const eics = eic[channel/32];
 
 uint32_t eic_mask   = (1UL << (channel % 32));
 eics->INTFLAG.reg = eic_mask; 
}

/** \brief extint_disable_irq Disables the transceiver main interrupt     

    \param [in] None
    \return None
 */
void extint_disable_irq(uint8_t channel)
{
 /* Get a pointer to the module hardware instance */
 Eic *const eic[EIC_INST_NUM] = EIC_INSTS;
 Eic *const eics = eic[channel/32];
 
 eics->INTENCLR.reg = (1 << channel);
}

/** \brief button_extint_enable_irq Enables the button interrupt     

    \param [in] None
    \return None
 */
void extint_enable_irq(uint8_t channel)
{
 /* Get a pointer to the module hardware instance */
 Eic *const eic[EIC_INST_NUM] = EIC_INSTS;
 Eic *const eics = eic[channel/32];
 
 eics->INTENSET.reg = (1 << channel);
}

/** \brief EIC_Handler External Interrupt handler function

     

    \param [in] None
    \return None
 */
void EIC_Handler(void)
{
 if (REG_EIC_INTFLAG & EIC_INTFLAG_EXTINT0)
 {
	 /* Clear the Flag */
	REG_EIC_INTFLAG =  EIC_INTFLAG_EXTINT0;
	ext_int0_callback();
 }
 if (REG_EIC_INTFLAG & EIC_INTFLAG_EXTINT8)
 {
	/* Clear the Flag */
	REG_EIC_INTFLAG =  EIC_INTFLAG_EXTINT8;
 }
}

