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

#ifndef samr21_extint_h__
#define samr21_extint_h__

#include "samr21_gpio.h"

/**
 * \brief External interrupt edge detection configuration enum.
 *
 * Enum for the possible signal edge detection modes of the External
 * Interrupt Controller module.
 */
enum extint_detect {
	/** No edge detection. Not allowed as a NMI detection mode on some
	 *  devices. */
	EXTINT_DETECT_NONE    = 0,
	/** Detect rising signal edges. */
	EXTINT_DETECT_RISING  = 1,
	/** Detect falling signal edges. */
	EXTINT_DETECT_FALLING = 2,
	/** Detect both signal edges. */
	EXTINT_DETECT_BOTH    = 3,
	/** Detect high signal levels. */
	EXTINT_DETECT_HIGH    = 4,
	/** Detect low signal levels. */
	EXTINT_DETECT_LOW     = 5,
};

typedef struct extint_cfg{
	uint8_t detect;
	uint8_t channel;
}extint_t;


void extint_flag_clear(uint8_t channel);
void extint_disable_irq(uint8_t channel);
void extint_enable_irq(uint8_t channel);
void eic_irq_init(uint8_t pin, pinmux_t *pinmux, extint_t *extint);

#endif // samr21_extint_h__
