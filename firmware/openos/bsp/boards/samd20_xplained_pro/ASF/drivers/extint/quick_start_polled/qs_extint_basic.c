/**
 * \file
 *
 * \brief SAM D20/D21/R21 External Interrupt Driver Quick Start
 *
 * Copyright (C) 2013-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#include <asf.h>

void configure_extint_channel(void);

//! [setup]
void configure_extint_channel(void)
{
//! [setup_1]
	struct extint_chan_conf config_extint_chan;
//! [setup_1]
//! [setup_2]
	extint_chan_get_config_defaults(&config_extint_chan);
//! [setup_2]

//! [setup_3]
	config_extint_chan.gpio_pin           = BUTTON_0_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = BUTTON_0_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
//! [setup_3]
//! [setup_4]
	extint_chan_set_config(BUTTON_0_EIC_LINE, &config_extint_chan);
//! [setup_4]
}
//! [setup]

int main(void)
{
	system_init();

	//! [setup_init]
	configure_extint_channel();
	//! [setup_init]

	//! [main]
	while (true) {
		//! [main_1]
		if (extint_chan_is_detected(BUTTON_0_EIC_LINE)) {
		//! [main_1]

		//! [main_2]
			// Do something in response to EXTINT edge detection
			bool button_pin_state = port_pin_get_input_level(BUTTON_0_PIN);
			port_pin_set_output_level(LED_0_PIN, button_pin_state);
		//! [main_2]

		//! [main_3]
			extint_chan_clear_detected(BUTTON_0_EIC_LINE);
		//! [main_3]
		}
	}
	//! [main]
}
