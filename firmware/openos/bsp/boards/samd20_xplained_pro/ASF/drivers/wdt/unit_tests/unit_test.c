/**
 * \file
 *
 * \brief SAM D20/D21/R21 Watchdog Unit test
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
 * \mainpage SAM D20/D21/R21 WDT Unit Test
 * See \ref appdoc_main "here" for project documentation.
 * \copydetails appdoc_preface
 *
 *
 * \page appdoc_preface Overview
 * This unit test carries out tests for WDT driver.
 * It consists of test cases for the following functionalities:
 *      - Test for driver initialization.
 *      - Test for early warning callbacks.
 *      - Test for system reset upon timeout.
 */

/**
 * \page appdoc_main SAM D20/D21/R21 WDT Unit Test
 *
 * Overview:
 * - \ref appdoc_sam0_wdt_unit_test_intro
 * - \ref appdoc_sam0_wdt_unit_test_setup
 * - \ref appdoc_sam0_wdt_unit_test_usage
 * - \ref appdoc_sam0_wdt_unit_test_compinfo
 * - \ref appdoc_sam0_wdt_unit_test_contactinfo
 *
 * \section appdoc_sam0_wdt_unit_test_intro Introduction
 * \copydetails appdoc_preface
 *
 * The following kit is required for carrying out the test:
 *      - SAM D20 Xplained Pro board
 *      - SAM D21 Xplained Pro board
 *      - SAM R21 Xplained Pro board
 *
 * \section appdoc_sam0_wdt_unit_test_setup Setup
 * The following connections has to be made using wires:
 *  - None
 *
 * To run the test:
 *  - Connect the SAM D20/D21/R21 Xplained Pro board to the computer using a
 *    micro USB cable.
 *  - Open the virtual COM port in a terminal application.
 *    \note The USB composite firmware running on the Embedded Debugger (EDBG)
 *          will enumerate as debugger, virtual COM port and EDBG data
 *          gateway.
 *  - Build the project, program the target and run the application.
 *    The terminal shows the results of the unit test.
 *
 * \section appdoc_sam0_wdt_unit_test_usage Usage
 *  - The unit tests are carried out using the internal Watchdog.
 *
 * \section appdoc_sam0_wdt_unit_test_compinfo Compilation Info
 * This software was written for the GNU GCC and IAR for ARM.
 * Other compilers may or may not work.
 *
 * \section appdoc_sam0_wdt_unit_test_contactinfo Contact Information
 * For further information, visit
 * <a href="http://www.atmel.com">http://www.atmel.com</a>.
 */

#include <asf.h>
#include <stdio_serial.h>
#include <string.h>
#include "conf_test.h"

/* Structure for UART module connected to EDBG (used for unit test output) */
struct usart_module cdc_uart_module;

/* Flag that indicates whether Watchdog was Reset cause */
static volatile bool wdr_flag = false;

/**
 * \brief Test Early Warning of Watchdog module
 *
 * If last reset cause was not Watchdog, following the Watchdog initialization
 * and enabling in the previous test, this function will wait for
 * CONF_WDT_EARLY_WARNING_WAIT_MS and will check if early warning flag is set.
 * Consequently, clear the early warning flag.
 *
 * \param test Current test case.
 */
static void run_wdt_early_warning_test(const struct test_case *test)
{
	/* Check if last reset was by Watchdog module */
	if (wdr_flag == false) {
		/* Wait for Early Warning flag to be set */
		delay_ms(CONF_WDT_EARLY_WARNING_WAIT_MS);

		/* Check if the Early Warning flag is set */
		test_assert_true(test, wdt_is_early_warning() == true,
				"Early Warning failed \n");

		/* Clear the Early Warning flag */
		wdt_clear_early_warning();
	}
}

/**
 * \brief Wait for Watchdog module to reset
 *
 *  Wait for CONF_WDT_RESET_WAIT_MS for Watchdog to reset the device
 *
 * \param test Current test case.
 */
static void wait_for_wdt_reset(const struct test_case *test)
{
	/* Check if last reset was by Watchdog module */
	if (wdr_flag == false) {
		/* Wait for Watchdog module to reset the device */
		delay_ms(CONF_WDT_RESET_WAIT_MS);
	}
}

/**
 * \brief Test whether last reset was by Watchdog module
 *
 * This function tests whether the last reset was caused by timeout
 * of Watchdog module
 *
 * \param test Current test case.
 */
static void run_reset_cause_test(const struct test_case *test)
{
	test_assert_true(test, wdr_flag == true,
			"Watchdog reset failed: %x", wdr_flag);
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
 * \brief Run WDT unit tests
 *
 * Initializes the system and serial output, then sets up the
 * WDT unit test suite and runs it.
 */
int main(void)
{
	/* Check whether reset cause was Watchdog */
	wdr_flag = (system_get_reset_cause() & PM_RCAUSE_WDT);
	system_init();

	/* Reset the Watchdog count */
	wdt_reset_count();

	struct wdt_conf config_wdt;
	/* Get the Watchdog default configuration */
	wdt_get_config_defaults(&config_wdt);
	if(wdr_flag) {
		config_wdt.enable = false;
	}
	/* Set the desired configuration */
	config_wdt.clock_source         = CONF_WDT_GCLK_GEN;
	config_wdt.timeout_period       = CONF_WDT_TIMEOUT_PERIOD;
	config_wdt.early_warning_period = CONF_WDT_EARLY_WARNING_PERIOD;
	wdt_set_config(&config_wdt);

	cdc_uart_init();

	DEFINE_TEST_CASE(wdt_early_warning_test, NULL,
			run_wdt_early_warning_test, wait_for_wdt_reset,
			"WDT Early Warning Test");

	DEFINE_TEST_CASE(reset_cause_test, NULL,
			run_reset_cause_test, NULL,
			"Confirming Watchdog Reset");

	/* Put test case addresses in an array */
	DEFINE_TEST_ARRAY(wdt_tests) = {
			&wdt_early_warning_test,
			&reset_cause_test,
			};

	/* Define the test suite */
	DEFINE_TEST_SUITE(wdt_suite, wdt_tests,
			"SAM D20/D21/R21 WDT driver test suite");

	/* Run all tests in the suite*/
	test_suite_run(&wdt_suite);

	while (1) {
		/* Intentionally left empty */
	}

}
