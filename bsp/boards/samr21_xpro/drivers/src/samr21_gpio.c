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
#include "samr21_gpio.h"



/*
 * \brief set the port pin direction and configuration.
 *        This will configure the particular pin will be input or output
 *       
 * 
 * \param[in] pin pin number
 * \param[in] dir output/input
 *
 */
void port_config(uint8_t pin, uint8_t dir)
{
  pinmux_t pinmux;
  pinmux.dir = dir;
  pinmux.mux_loc = SYSTEM_PINMUX_GPIO;
  pinmux.pull = PORT_PIN_PULLNONE;
  pinmux_config(pin, &pinmux);
}

/** \brief pinmux_config Configures the given pin to given functionality

     MUX will provides an option to select the various peripheral selection

    \param [in] pin 
	\param [in] config
    \return None
 */
void pinmux_config(const uint8_t pin, pinmux_t *config)
{	
   Port *const ports[PORT_INST_NUM] = PORT_INSTS;
   PortGroup *const port = &(ports[(pin/128)]->Group[(pin / 32)]);
   uint32_t pin_mask = (1UL << (pin % 32));
 
   /* Track the configuration bits into a temporary variable before writing */
	uint32_t pin_cfg = 0;
	if (config->mux_loc != SYSTEM_PINMUX_GPIO)
	{
		pin_cfg |= PORT_WRCONFIG_PMUXEN;
		pin_cfg |= (config->mux_loc << PORT_WRCONFIG_PMUX_Pos);
	}
	
	if (config->dir == PORT_PIN_DIR_OUTPUT)
	{
		/* Cannot use a pullup if the output driver is enabled,
			 * if requested the input buffer can only sample the current
			 * output state */
		pin_cfg &= ~PORT_WRCONFIG_PULLEN;
	} 
	else
	{
	 /* Enable input buffer flag */
	 pin_cfg |= PORT_WRCONFIG_INEN;
	 if (config->pull)
	 {
		 pin_cfg |= PORT_WRCONFIG_PULLEN;
	 }
	 /* Clear the port DIR bits to disable the output buffer */
	 port->DIRCLR.reg = pin_mask; 				
	}	

	/* The Write Configuration register (WRCONFIG) requires the
	 * pins to to grouped into two 16-bit half-words - split them out here */
	uint32_t lower_pin_mask = (pin_mask & 0xFFFF);
	uint32_t upper_pin_mask = (pin_mask >> 16);

	/* Configure the lower 16-bits of the port to the desired configuration,
	 * including the pin peripheral multiplexer just in case it is enabled */
	port->WRCONFIG.reg
		= (lower_pin_mask << PORT_WRCONFIG_PINMASK_Pos) |
			pin_cfg | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_WRPINCFG;

	/* Configure the upper 16-bits of the port to the desired configuration,
	 * including the pin peripheral multiplexer just in case it is enabled */
	port->WRCONFIG.reg
		= (upper_pin_mask << PORT_WRCONFIG_PINMASK_Pos) |
			pin_cfg | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_WRPINCFG |
			PORT_WRCONFIG_HWSEL;

	if (pin_cfg & PORT_WRCONFIG_PULLEN) 
	{
		/* Set the OUT register bits to enable the pullup if requested,
			* clear to enable pull-down */
		if (config->pull == PORT_PIN_PULLUP) 
		{
			port->OUTSET.reg = pin_mask;
		} 
		else 
		{
			port->OUTCLR.reg = pin_mask;
		}
	}

	/* Check if the user has requested that the output buffer be enabled */
	if (config->dir == PORT_PIN_DIR_OUTPUT)
	{
		/* Set the port DIR bits to enable the output buffer */
		port->DIRSET.reg = pin_mask;
	}
}
