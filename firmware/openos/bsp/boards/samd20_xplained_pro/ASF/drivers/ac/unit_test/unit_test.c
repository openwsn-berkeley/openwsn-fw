/**
 * \file
 *
 * \brief SAM D20/D21 Analog Comparator (AC) Unit test
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

/**
 * \mainpage SAM D20/D21 AC Unit Test
 * See \ref appdoc_main "here" for project documentation.
 * \copydetails appdoc_preface
 *
 *
 * \page appdoc_preface Overview
 * This unit test carries out tests for the AC driver.
 * It consists of test cases for the following functionalities:
 *      - Test for AC initialization.
 *      - Test for AC single shot comparison.
 *      - Test for AC callback mode comparison.
 *      - Test for AC continuous window mode comparison.
 */

/**
 * \page appdoc_main SAM D20/D21 AC Unit Test
 *
 * Overview:
 * - \ref appdoc_sam0_ac_unit_test_intro
 * - \ref appdoc_sam0_ac_unit_test_setup
 * - \ref appdoc_sam0_ac_unit_test_usage
 * - \ref appdoc_sam0_ac_unit_test_compinfo
 * - \ref appdoc_sam0_ac_unit_test_contactinfo
 *
 * \section appdoc_sam0_ac_unit_test_intro Introduction
 * \copydetails appdoc_preface
 *
 * Input to the AC is provided with the DAC module.
 *
 * The following kit is required for carrying out the test:
 *      - SAM D20 Xplained Pro board
 *      - SAM D21 Xplained Pro board
 *
 * \section appdoc_sam0_ac_unit_test_setup Setup
 * The following connections has to be made using wires:
 *  - \b DAC VOUT (PA02) <-----> AIN0 (PA04)
 *
 * To run the test:
 *  - Connect the SAM D20/D21 Xplained Pro board to the computer using a
 *    micro USB cable.
 *  - Open the virtual COM port in a terminal application.
 *    \note The USB composite firmware running on the Embedded Debugger (EDBG)
 *          will enumerate as debugger, virtual COM port and EDBG data
 *          gateway.
 *  - Build the project, program the target and run the application.
 *    The terminal shows the results of the unit test.
 *
 * \section appdoc_sam0_ac_unit_test_usage Usage
 *  - The unit test configures DAC module to provide voltage to the AC positive
 *    input.
 *  - AC negative input is given from internal voltage scaler.
 *  - DAC output is adjusted to generate various voltages which are compared by
 *    the AC.
 *  - Different modes of the AC are tested.
 *
 * \section appdoc_sam0_ac_unit_test_compinfo Compilation Info
 * This software was written for the GNU GCC and IAR for ARM.
 * Other compilers may or may not work.
 *
 * \section appdoc_sam0_ac_unit_test_contactinfo Contact Information
 * For further information, visit
 * <a href="http://www.atmel.com">http://www.atmel.com</a>.
 */

#include <asf.h>
#include <stdio_serial.h>
#include <string.h>
#include "conf_test.h"

#define AC_SCALER_0_25_VOLT 4
#define AC_SCALER_0_50_VOLT 9
#define AC_SCALER_0_75_VOLT 14

/* Theoretical DAC value for 0.0V output*/
#define DAC_VAL_ZERO_VOLT   0
/* Theoretical DAC value for 0.5V output*/
#define DAC_VAL_HALF_VOLT   512
/* Theoretical DAC value for 1.0V output*/
#define DAC_VAL_ONE_VOLT    1023

/* Structure for UART module connected to EDBG (used for unit test output) */
struct usart_module cdc_uart_module;
/* Structure for ADC module */
struct ac_module ac_inst;
/* Structure for DAC module */
struct dac_module dac_inst;

/* Interrupt flag used during callback test */
volatile bool interrupt_flag = false;
/* Flag to check successful initialization */
volatile bool ac_init_success = false;

/**
 * \internal
 * \brief AC callback function
 *
 * Called by AC driver on interrupt detection.
 *
 * \param module Pointer to the AC module (not used)
 */
static void ac_user_callback(struct ac_module *const module_inst)
{
	interrupt_flag = true;
}

/**
 * \brief Initialize the USART for unit test
 *
 * Initializes the SERCOM USART used for sending the unit test status to the
 * computer via the EDBG CDC gateway.
 */
static void cdc_uart_init(void)
{
	struct usart_config usart_conf;

	/* Configure USART for unit test output */
	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = CONF_STDIO_MUX_SETTING;
	usart_conf.pinmux_pad0 = CONF_STDIO_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = CONF_STDIO_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = CONF_STDIO_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = CONF_STDIO_PINMUX_PAD3;
	usart_conf.baudrate    = CONF_STDIO_BAUDRATE;

	stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART, &usart_conf);
	usart_enable(&cdc_uart_module);
}

/**
 * \brief Initialize the DAC for unit test
 *
 * Initializes the DAC module used for sending the analog values
 * to the AC during test.
 */
static void test_dac_init(void)
{
	/* Structures for DAC configuration */
	struct dac_config config;
	struct dac_chan_config chan_config;

	/* Configure the DAC module */
	dac_get_config_defaults(&config);
	dac_init(&dac_inst, DAC, &config);
	dac_enable(&dac_inst);

	/* Configure the DAC channel */
	dac_chan_get_config_defaults(&chan_config);
	dac_chan_set_config(&dac_inst, DAC_CHANNEL_0, &chan_config);
	dac_chan_enable(&dac_inst, DAC_CHANNEL_0);
}

/**
 * \internal
 * \brief Test for AC initialization.
 *
 * This test initializes the AC module and checks whether the
 * initialization is successful or not.
 *
 * If this test fails no other tests will run.
 *
 * \param test Current test case.
 */
static void run_ac_init_test(const struct test_case *test)
{
	enum status_code status = STATUS_ERR_IO;

	/* Structure for AC configuration */
	struct ac_config config;
	struct ac_chan_config channel_config;

	ac_get_config_defaults(&config);
	/* Initialize the AC */
	status = ac_init(&ac_inst, AC, &config);
	/* Check for successful initialization */
	test_assert_true(test, status == STATUS_OK,
			"AC initialization failed");

	status = STATUS_ERR_IO;
	ac_chan_get_config_defaults(&channel_config);
	channel_config.sample_mode       = AC_CHAN_MODE_SINGLE_SHOT;
	channel_config.positive_input    = AC_CHAN_POS_MUX_PIN0;
	channel_config.negative_input    = AC_CHAN_NEG_MUX_SCALED_VCC;
	channel_config.vcc_scale_factor  = AC_SCALER_0_50_VOLT;

	struct system_pinmux_config ac0_pin_conf;
	system_pinmux_get_config_defaults(&ac0_pin_conf);
	ac0_pin_conf.direction    = SYSTEM_PINMUX_PIN_DIR_INPUT;
	ac0_pin_conf.mux_position = MUX_PA04B_AC_AIN0;
	system_pinmux_pin_set_config(PIN_PA04B_AC_AIN0, &ac0_pin_conf);

	/* Set the channel configuration */
	status
		= ac_chan_set_config(&ac_inst, AC_CHAN_CHANNEL_0,
			&channel_config);
	/* Check for successful configuration */
	test_assert_true(test, status == STATUS_OK,
			"AC channel configuration failed");
	/* Enable the AC channel & AC module */
	ac_chan_enable(&ac_inst, AC_CHAN_CHANNEL_0);
	ac_enable(&ac_inst);

	if (status == STATUS_OK) {
		ac_init_success = true;
	}
}

/**
 * \internal
 * \brief Test for AC comparison in single shot mode.
 *
 * This test checks the single shot comparison of the AC.
 * 0.5V is applied to the negative input of AC from internal voltage scaler.
 * 0V and 1V from DAC is applied to the positive input and the results
 * are verified.
 *
 * \param test Current test case.
 */
static void run_ac_single_shot_test(const struct test_case *test)
{
	volatile uint32_t state = AC_CHAN_STATUS_UNKNOWN;

	/* Skip test if initialization failed */
	test_assert_true(test, ac_init_success,
			"Skipping test due to failed AC initialization");

	/* Test for positive input < negative input */
	dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_ZERO_VOLT);
	delay_ms(1);
	ac_chan_trigger_single_shot(&ac_inst, AC_CHAN_CHANNEL_0);
	while (!ac_chan_is_ready(&ac_inst, AC_CHAN_CHANNEL_0)) {
	}
	state = ac_chan_get_status(&ac_inst, AC_CHAN_CHANNEL_0);
	state = state & AC_CHAN_STATUS_NEG_ABOVE_POS;
	test_assert_true(test, state == AC_CHAN_STATUS_NEG_ABOVE_POS,
			"AC comparison failed: POS < NEG not detected");

	/* Test for negative input < positive input */
	state = AC_CHAN_STATUS_UNKNOWN;
	dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_ONE_VOLT);
	delay_ms(1);
	ac_chan_trigger_single_shot(&ac_inst, AC_CHAN_CHANNEL_0);
	while (!ac_chan_is_ready(&ac_inst, AC_CHAN_CHANNEL_0)) {
	}
	state = ac_chan_get_status(&ac_inst, AC_CHAN_CHANNEL_0);
	state = state & AC_CHAN_STATUS_POS_ABOVE_NEG;
	test_assert_true(test, state == AC_CHAN_STATUS_POS_ABOVE_NEG,
			"AC comparison failed: Interrupt not detected");
	ac_chan_clear_status(&ac_inst, AC_CHAN_CHANNEL_0);
}

/**
 * \internal
 * \brief Setup Function: AC callback mode test.
 *
 * This function initializes the AC to generate interrupt when the AC
 * output toggles.
 *
 * \param test Current test case.
 */
static void setup_ac_callback_mode_test(const struct test_case *test)
{
	enum status_code status = STATUS_ERR_IO;

	/* Structure for AC configuration */
	struct ac_config config;
	struct ac_chan_config channel_config;

	/* Set input to 0V */
	dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_ZERO_VOLT);

	/* Set the flag to false */
	ac_init_success = false;
	ac_reset(&ac_inst);

	ac_get_config_defaults(&config);
	/* Initialize the AC */
	status = ac_init(&ac_inst, AC, &config);
	/* Check for successful initialization */
	test_assert_true(test, status == STATUS_OK,
			"AC initialization failed");

	/* Configure the AC input pin */
	struct system_pinmux_config ac0_pin_conf;
	system_pinmux_get_config_defaults(&ac0_pin_conf);
	ac0_pin_conf.direction    = SYSTEM_PINMUX_PIN_DIR_INPUT;
	ac0_pin_conf.mux_position = MUX_PA04B_AC_AIN0;
	system_pinmux_pin_set_config(PIN_PA04B_AC_AIN0, &ac0_pin_conf);

	/* Channel configuration */
	status = STATUS_ERR_IO;
	ac_chan_get_config_defaults(&channel_config);
	channel_config.sample_mode       = AC_CHAN_MODE_CONTINUOUS;
	channel_config.positive_input    = AC_CHAN_POS_MUX_PIN0;
	channel_config.negative_input    = AC_CHAN_NEG_MUX_SCALED_VCC;
	channel_config.vcc_scale_factor  = AC_SCALER_0_50_VOLT;
	status
		= ac_chan_set_config(&ac_inst, AC_CHAN_CHANNEL_0,
			&channel_config);
	/* Check for successful initialization */
	test_assert_true(test, status == STATUS_OK,
			"AC channel initialization failed");

	/* Callback configuration */
	status = ac_register_callback(&ac_inst, ac_user_callback,
			AC_CALLBACK_COMPARATOR_0);
	test_assert_true(test, status == STATUS_OK,
			"AC callback registration failed");

	/* Enable the AC channel & the module */
	ac_chan_enable(&ac_inst, AC_CHAN_CHANNEL_0);
	ac_enable(&ac_inst);
	ac_enable_callback(&ac_inst, AC_CALLBACK_COMPARATOR_0);

	if (status == STATUS_OK) {
		ac_init_success = true;
	}
}

/**
 * \internal
 * \brief AC callback mode test function
 *
 * This test changes the positive input from 0V to 1V to detect the
 * rising edge and again from 1V to 0V to detect the falling edge.
 *
 * \param test Current test case.
 */
static void run_ac_callback_mode_test(const struct test_case *test)
{
	uint16_t timeout_cycles = 100;

	/* Skip test if initialization failed */
	test_assert_true(test, ac_init_success,
			"Skipping test due to failed AC initialization");

	/* Test for rising edge detection */
	dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_ONE_VOLT);
	delay_ms(1);
	do {
		timeout_cycles--;
		if (interrupt_flag) {
			break;
		}
	} while (timeout_cycles > 0);

	test_assert_true(test, timeout_cycles,
			"Error: Timeout in rising edge detection");

	/* Test for falling edge detection */
	timeout_cycles = 100;
	interrupt_flag = false;
	dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_ZERO_VOLT);
	delay_ms(1);
	do {
		timeout_cycles--;
		if (interrupt_flag) {
			break;
		}
	} while (timeout_cycles > 0);

	test_assert_true(test, timeout_cycles,
			"Error: Timeout in falling edge detection");
}

/**
 * \internal
 * \brief Cleanup Function: AC callback mode test.
 *
 * This function unregisters & disables callback for edge detection.
 *
 * \param test Current test case.
 */
static void cleanup_ac_callback_mode_test(const struct test_case *test)
{
	ac_disable_callback(&ac_inst, AC_CALLBACK_COMPARATOR_0);
	ac_unregister_callback(&ac_inst, AC_CALLBACK_COMPARATOR_0);
}

/**
 * \internal
 * \brief Setup Function: AC window mode test.
 *
 * This function initializes the AC in window mode detection.
 * 0.25V and 0.75V from internal voltage scaler are used as lower
 * and upper limits of the window respectively.
 *
 * \param test Current test case.
 */
static void setup_ac_window_mode_test(const struct test_case *test)
{
	enum status_code status = STATUS_ERR_IO;

	/* Structure for AC configuration */
	struct ac_config config;
	struct ac_chan_config channel_config;
	struct ac_win_config window_config;

	/* Set the flag to false */
	ac_init_success = false;
	ac_reset(&ac_inst);

	ac_get_config_defaults(&config);
	/* Initialize the AC */
	status = ac_init(&ac_inst, AC, &config);
	/* Check for successful initialization */
	test_assert_true(test, status == STATUS_OK,
			"AC initialization failed");

	/* Configure the AC input pin */
	struct system_pinmux_config ac0_pin_conf;
	system_pinmux_get_config_defaults(&ac0_pin_conf);
	ac0_pin_conf.direction    = SYSTEM_PINMUX_PIN_DIR_INPUT;
	ac0_pin_conf.mux_position = MUX_PA04B_AC_AIN0;
	system_pinmux_pin_set_config(PIN_PA04B_AC_AIN0, &ac0_pin_conf);

	/* Channel configuration */
	status = STATUS_ERR_IO;
	ac_chan_get_config_defaults(&channel_config);
	channel_config.sample_mode       = AC_CHAN_MODE_SINGLE_SHOT;
	channel_config.positive_input    = AC_CHAN_POS_MUX_PIN0;
	channel_config.negative_input    = AC_CHAN_NEG_MUX_SCALED_VCC;
	channel_config.vcc_scale_factor  = AC_SCALER_0_25_VOLT;

	/* Set the channel configuration for CHAN1 - Lower limit*/
	status
		= ac_chan_set_config(&ac_inst, AC_CHAN_CHANNEL_1,
			&channel_config);
	/* Check for successful initialization */
	test_assert_true(test, status == STATUS_OK,
			"AC channel 1 initialization failed");
	/* Set the channel configuration for CHAN0 - Upper limit*/
	channel_config.vcc_scale_factor  = AC_SCALER_0_75_VOLT;
	status = ac_chan_set_config(&ac_inst, AC_CHAN_CHANNEL_0, &channel_config);
	/* Check for successful initialization */
	test_assert_true(test, status == STATUS_OK,
			"AC channel 0 initialization failed");

	/* Window mode configuration */
	status = STATUS_ERR_IO;
	ac_win_get_config_defaults(&window_config);
	status = ac_win_set_config(&ac_inst, AC_WIN_CHANNEL_0, &window_config);
	/* Check for successful initialization */
	test_assert_true(test, status == STATUS_OK,
			"AC window mode initialization failed");

	/* Enable the AC channels */
	ac_chan_enable(&ac_inst, AC_CHAN_CHANNEL_0);
	ac_chan_enable(&ac_inst, AC_CHAN_CHANNEL_1);
	/* Enable window mode */
	status = ac_win_enable(&ac_inst, AC_WIN_CHANNEL_0);
	/* Check for successful initialization */
	test_assert_true(test, status == STATUS_OK,
			"AC window mode enable failed");
	/* Enable AC module */
	ac_enable(&ac_inst);

	if (status == STATUS_OK) {
		ac_init_success = true;
	}
}

/**
 * \internal
 * \brief AC window mode test function
 *
 * This test checks the window functionality of the AC module.
 * Inputs are given in each region of the window (below, inside & above)
 * and corresponding window output states are verified.
 *
 * \param test Current test case.
 */
static void run_ac_window_mode_test(const struct test_case *test)
{
	volatile uint32_t state = AC_WIN_STATUS_UNKNOWN;

	/* Skip test if initialization failed */
	test_assert_true(test, ac_init_success,
			"Skipping test due to failed AC initialization");

	/* Test for region-below detection */
	dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_ZERO_VOLT);
	delay_ms(1);
	ac_chan_trigger_single_shot(&ac_inst, AC_CHAN_CHANNEL_0);
	while (!ac_win_is_ready(&ac_inst, AC_WIN_CHANNEL_0)) {
	}
	state = ac_win_get_status(&ac_inst, AC_WIN_CHANNEL_0);
	state = state & AC_WIN_STATUS_BELOW;
	test_assert_true(test, state == AC_WIN_STATUS_BELOW,
			"AC window mode: Less than lower limit not detected");
	ac_win_clear_status(&ac_inst, AC_WIN_CHANNEL_0);
	ac_chan_clear_status(&ac_inst, AC_CHAN_CHANNEL_0);
	ac_chan_clear_status(&ac_inst, AC_CHAN_CHANNEL_1);

	/* Test for region-inside detection */
	state = AC_WIN_STATUS_UNKNOWN;
	dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_HALF_VOLT);
	delay_ms(1);
	ac_chan_trigger_single_shot(&ac_inst, AC_CHAN_CHANNEL_0);
	while (!ac_win_is_ready(&ac_inst, AC_WIN_CHANNEL_0)) {
	}
	state = ac_win_get_status(&ac_inst, AC_WIN_CHANNEL_0);
	state = state & AC_WIN_STATUS_INSIDE;
	test_assert_true(test, state == AC_WIN_STATUS_INSIDE,
			"AC window mode: Within limit not detected");
	ac_win_clear_status(&ac_inst, AC_WIN_CHANNEL_0);
	ac_chan_clear_status(&ac_inst, AC_CHAN_CHANNEL_0);
	ac_chan_clear_status(&ac_inst, AC_CHAN_CHANNEL_1);

	/* Test for region-above detection */
	state = AC_WIN_STATUS_UNKNOWN;
	dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_ONE_VOLT);
	delay_ms(1);
	ac_chan_trigger_single_shot(&ac_inst, AC_CHAN_CHANNEL_0);
	while (!ac_win_is_ready(&ac_inst, AC_WIN_CHANNEL_0)) {
	}
	state = ac_win_get_status(&ac_inst, AC_WIN_CHANNEL_0);
	state = state & AC_WIN_STATUS_ABOVE;
	test_assert_true(test, state == AC_WIN_STATUS_ABOVE,
			"AC window mode: More than upper limit not detected");
	ac_win_clear_status(&ac_inst, AC_WIN_CHANNEL_0);
	ac_chan_clear_status(&ac_inst, AC_CHAN_CHANNEL_0);
	ac_chan_clear_status(&ac_inst, AC_CHAN_CHANNEL_1);
}

/**
 * \brief Run AC unit tests
 *
 * Initializes the system and serial output, then sets up the
 * AC unit test suite and runs it.
 */
int main(void)
{
	system_init();
	delay_init();
	cdc_uart_init();
	test_dac_init();

	/* Define Test Cases */
	DEFINE_TEST_CASE(ac_init_test, NULL,
			run_ac_init_test, NULL,
			"Testing Analog Comparator Module Initialization");

	DEFINE_TEST_CASE(ac_single_shot_test, NULL,
			run_ac_single_shot_test, NULL,
			"Testing AC single shot comparison");

	DEFINE_TEST_CASE(ac_callback_mode_test,
			setup_ac_callback_mode_test,
			run_ac_callback_mode_test,
			cleanup_ac_callback_mode_test,
			"Testing AC callback mode");

	DEFINE_TEST_CASE(ac_window_mode_test,
			setup_ac_window_mode_test,
			run_ac_window_mode_test, NULL,
			"Testing AC window mode with single shot");

	/* Put test case addresses in an array */
	DEFINE_TEST_ARRAY(ac_tests) = {
		&ac_init_test,
		&ac_single_shot_test,
		&ac_callback_mode_test,
		&ac_window_mode_test,
	};

	/* Define the test suite */
	DEFINE_TEST_SUITE(ac_test_suite, ac_tests,
			"SAM D20/D21 AC driver test suite");

	/* Run all tests in the suite*/
	test_suite_run(&ac_test_suite);

	while (true) {
		/* Intentionally left empty */
	}
}
