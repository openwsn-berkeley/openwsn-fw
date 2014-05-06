/**
 * \file
 *
 * \brief SAM D20/D21/R21 GPIO Port Driver Quick Start
 *
 * Copyright (C) 2012-2014 Atmel Corporation. All rights reserved.
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

void configure_port_pins(void);

//! [setup]
void configure_port_pins(void)
{
//! [setup_1]
	struct port_config config_port_pin;
//! [setup_1]
//! [setup_2]
	port_get_config_defaults(&config_port_pin);
//! [setup_2]

//! [setup_3]
	config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
//! [setup_3]
//! [setup_4]
	port_pin_set_config(BUTTON_0_PIN, &config_port_pin);
//! [setup_4]

//! [setup_5]
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
//! [setup_5]
//! [setup_6]
	port_pin_set_config(LED_0_PIN, &config_port_pin);
//! [setup_6]
}
//! [setup]

int main(void)
{
	system_init();

	//! [setup_init]
	configure_port_pins();
	//! [setup_init]

	//! [main]
	while (true) {
		//! [main_1]
		bool pin_state = port_pin_get_input_level(BUTTON_0_PIN);
		//! [main_1]

		//! [main_2]
		port_pin_set_output_level(LED_0_PIN, !pin_state);
		//! [main_2]
	}
	//! [main]
}
