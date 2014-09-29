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

#ifndef samr21_gpio_h__
#define samr21_gpio_h__

#include "sam.h"
#include "base_type.h"

#define SYSTEM_PINMUX_GPIO    (1 << 7)

#define SET_HIGH         true
#define SET_LOW          false

typedef struct pinmux_cfg{
	uint32_t mux_loc;
	uint8_t dir;
	uint8_t pull;
}pinmux_t;

/**
 * \brief Port pin direction configuration enum.
 *
 * Enum for the possible pin direction settings of the port pin configuration
 * structure, to indicate the direction the pin should use.
 */
enum system_pinmux_pin_dir {
	/** The pin's input buffer should be enabled, so that the pin state can
	 *  be read. */
	PORT_PIN_DIR_INPUT,
	/** The pin's output buffer should be enabled, so that the pin state can
	 *  be set (but not read back). */
	PORT_PIN_DIR_OUTPUT,
	/** The pin's output and input buffers should both be enabled, so that the
	 *  pin state can be set and read back. */
	PORT_PIN_DIR_OUTPUT_WTH_READBACK,
};

enum system_pinmux_pin_pull {
	PORT_PIN_PULLNONE,
	PORT_PIN_PULLUP,
	PORT_PIN_PULLDOWN
};

/**
 *  \brief Sets the state of a port pin that is configured as an output.
 *
 *  Sets the current output level of a port pin to a given logic level.
 *
 *  \param[in] pin  Index of the GPIO pin to write to.
 *  \param[in] level Logical level to set the given pin to.
 */
static inline void port_pin_set_level(uint8_t pin, uint8_t level)
{
	/* Array of available ports. */
	Port *const ports[PORT_INST_NUM] = PORT_INSTS;
	PortGroup *const port = &(ports[(pin/128)]->Group[(pin / 32)]);
	uint32_t pin_mask = (1UL << (pin % 32));
	
	if (level)
	{
		port->OUTSET.reg = pin_mask;
	} 
	else
	{
		port->OUTCLR.reg = pin_mask;
	}
}

/**
 *  \brief Toggles the state of a port pin that is configured as an output.
 *
 *  Toggles the current output level of a port pin.
 *
 *  \param[in] pin  Index of the GPIO pin to toggle.
 */
static inline void port_pin_toggle(uint8_t pin)
{
	/* Array of available ports. */
	Port *const ports[PORT_INST_NUM] = PORT_INSTS;
	PortGroup *const port = &(ports[(pin/128)]->Group[(pin / 32)]);
	uint32_t pin_mask = (1UL << (pin % 32));
	port->OUTTGL.reg = pin_mask;
}

void port_config(uint8_t pin, uint8_t dir);
void pinmux_config(const uint8_t pin, pinmux_t *config);

#endif // samr21_gpio_h__
