/**
 * \file
 *
 * \brief SAM D21/R21 TCC - Timer Counter for Control Applications Callback Driver Quick Start
 *
 * Copyright (C) 2013-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
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
#include <conf_quick_start_callback.h>

static void _configure_tcc(void);
static void _configure_tcc_callbacks(void);
static void _tcc_callback_to_change_duty_cycle(
		struct tcc_module *const module_inst);


//! [module_inst]
struct tcc_module tcc_instance;
//! [module_inst]

//! [callback_funcs]
static void _tcc_callback_to_change_duty_cycle(
		struct tcc_module *const module_inst)
{
	static uint32_t delay = 10;
	static uint32_t i = 0;

	if (--delay) {
		return;
	}
	delay = 10;
	i = (i + 0x0800) & 0xFFFF;
	tcc_set_compare_value(module_inst,
			TCC_MATCH_CAPTURE_CHANNEL_0 + CONF_PWM_CHANNEL, i + 1);
}
//! [callback_funcs]

//! [setup]
static void _configure_tcc(void)
{
	//! [setup_config]
	struct tcc_config config_tcc;
	//! [setup_config]
	//! [setup_config_defaults]
	tcc_get_config_defaults(&config_tcc, CONF_PWM_MODULE);
	//! [setup_config_defaults]

	//! [setup_change_config]
	config_tcc.counter.period = 0xFFFF;
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	config_tcc.compare.match[CONF_PWM_CHANNEL] = 0xFFFF;
	//! [setup_change_config]

	//! [setup_change_config_pwm]
	config_tcc.pins.enable_wave_out_pin[CONF_PWM_OUTPUT] = true;
	config_tcc.pins.wave_out_pin[CONF_PWM_OUTPUT]        = CONF_PWM_OUT_PIN;
	config_tcc.pins.wave_out_pin_mux[CONF_PWM_OUTPUT]    = CONF_PWM_OUT_MUX;
	//! [setup_change_config_pwm]

	//! [setup_set_config]
	tcc_init(&tcc_instance, CONF_PWM_MODULE, &config_tcc);
	//! [setup_set_config]

	//! [setup_enable]
	tcc_enable(&tcc_instance);
	//! [setup_enable]
}

static void _configure_tcc_callbacks(void)
{
	//! [setup_register_callback]
	tcc_register_callback(
			&tcc_instance,
			_tcc_callback_to_change_duty_cycle,
			TCC_CALLBACK_CHANNEL_0 + CONF_PWM_CHANNEL);
	//! [setup_register_callback]

	//! [setup_enable_callback]
	tcc_enable_callback(&tcc_instance,
			TCC_CALLBACK_CHANNEL_0 + CONF_PWM_CHANNEL);
	//! [setup_enable_callback]
}
//! [setup]

int main(void)
{
	system_init();

//! [setup_init]
	_configure_tcc();
	_configure_tcc_callbacks();
//! [setup_init]

//! [main]
	//! [enable_global_interrupts]
	system_interrupt_enable_global();
	//! [enable_global_interrupts]

	//! [main_loop]
	while (true) {
	}
	//! [main_loop]
//! [main]
}
