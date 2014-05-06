/**
 * \file
 *
 * \brief SAM D20/D21/R21 SPI Unit test
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
 * \mainpage SAM D20/D21/R21 SPI Unit Test
 * See \ref appdoc_main "here" for project documentation.
 * \copydetails appdoc_preface
 *
 *
 * \page appdoc_preface Overview
 * This unit test carries out tests for SERCOM SPI driver.
 * It consists of test cases for the following functionalities:
 *      - Test for driver initialization.
 *      - Test for single byte write and read by polling.
 *      - Test for buffer write by polling and read with interrupt.
 *      - Test for buffer read & write using transceive function.
 *      - Test for 9-bit data transfer.
 *      - Test for baudrate.
 */

/**
 * \page appdoc_main SAM D20/D21/R21 SPI Unit Test
 *
 * Overview:
 * - \ref appdoc_sam0_spi_unit_test_intro
 * - \ref appdoc_sam0_spi_unit_test_setup
 * - \ref appdoc_sam0_spi_unit_test_usage
 * - \ref appdoc_sam0_spi_unit_test_compinfo
 * - \ref appdoc_sam0_spi_unit_test_contactinfo
 *
 * \section appdoc_sam0_spi_unit_test_intro Introduction
 * \copydetails appdoc_preface
 *
 * The following kit is required for carrying out the test:
 *      - SAM D20 Xplained Pro board
 *      - SAM D21 Xplained Pro board
 *      - SAM R21 Xplained Pro board
 *
 * \section appdoc_sam0_spi_unit_test_setup Setup
 * The following connections has to be made using wires:
 * - SAM D21/D21 Xplained Pro
 *  - \b SS_0:  EXT1 PIN15 (PA05) <--> EXT2 PIN15 (PA17)
 *  - \b DO/DI: EXT1 PIN16 (PA06) <--> EXT2 PIN17 (PA16)
 *  - \b DI/DO: EXT1 PIN17 (PA04) <--> EXT2 PIN16 (PA18)
 *  - \b SCK:   EXT1 PIN18 (PA07) <--> EXT2 PIN18 (PA19)
 * - SAM R21 Xplained Pro
 *  - \b SS_0:  EXT1 PIN15 (PB03) <--> EXT1 PIN10 (PA23)
 *  - \b DO/DI: EXT1 PIN16 (PB22) <--> EXT1 PIN9  (PA22)
 *  - \b DI/DO: EXT1 PIN17 (PB02) <--> EXT1 PIN7  (PA18)
 *  - \b SCK:   EXT1 PIN18 (PB23) <--> EXT1 PIN8  (PA19)
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
 * \section appdoc_sam0_spi_unit_test_usage Usage
 *  - The unit tests are carried out with SERCOM1 on EXT2 as SPI master and
 *    SERCOM0 on EXT1 as SPI slave.
 *  - Data is transmitted from master to slave in lengths of a single byte
 *    as well as multiple bytes.
 *
 * \section appdoc_sam0_spi_unit_test_compinfo Compilation Info
 * This software was written for the GNU GCC and IAR for ARM.
 * Other compilers may or may not work.
 *
 * \section appdoc_sam0_spi_unit_test_contactinfo Contact Information
 * For further information, visit
 * <a href="http://www.atmel.com">http://www.atmel.com</a>.
 */

#include <asf.h>
#include <stdio_serial.h>
#include <string.h>
#include "conf_test.h"

/* Test Baud rate */
#define TEST_SPI_BAUDRATE             1000000UL

/* Buffer size used during test */
#define BUFFER_LENGTH                 256

/* Structures for SPI master, slave configuration & slave instance selection */
struct spi_module master;
struct spi_module slave;
struct spi_slave_inst slave_inst;

/* Structure for UART module connected to EDBG (used for unit test output) */
struct usart_module cdc_uart_module;

/* Transmit and receive buffers used by master during test */
uint8_t tx_buf[BUFFER_LENGTH];
uint8_t rx_buf[BUFFER_LENGTH];
/* Transmit and receive buffers used by slave during test */
uint8_t slave_tx_buf[BUFFER_LENGTH];
uint8_t slave_rx_buf[BUFFER_LENGTH];

/* Flag to indicate whether buffer transmission is complete or not */
volatile bool transfer_complete = false;
/* Flag to check whether initialization passed */
volatile bool spi_init_success = false;

/**
 * \internal
 * \brief SPI interrupt callback function
 *
 * Called by SPI driver when transmission/reception is complete.
 *
 * \param module SPI module causing the interrupt (not used)
 */
static void user_spi_callback(const struct spi_module *const module)
{
	transfer_complete = true;
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
 * \internal
 * \brief Test initializing SPI master & slave.
 *
 * This initializes the SPI master & slave.
 *
 * Test passes if initialization succeeds fails otherwise.
 *
 * \param test Current test case.
 */
static void run_spi_init_test(const struct test_case *test)
{
	enum status_code status = STATUS_ERR_IO;

	/* Structure for SPI configuration */
	struct spi_config config;
	struct spi_slave_inst_config slave_config;

	/* Select SPI slave SS pin */
	spi_slave_inst_get_config_defaults(&slave_config);
	slave_config.ss_pin = CONF_SPI_SLAVE_SS_PIN;
	spi_attach_slave(&slave_inst, &slave_config);

	/* Configure the SPI master */
	spi_get_config_defaults(&config);
	config.mux_setting     = CONF_SPI_MASTER_SPI_MUX;
	config.pinmux_pad0     = CONF_SPI_MASTER_DATA_IN_PIN_MUX;
	config.pinmux_pad1     = PINMUX_UNUSED;
	config.pinmux_pad2     = CONF_SPI_MASTER_DATA_OUT_PIN_MUX;
	config.pinmux_pad3     = CONF_SPI_MASTER_SCK_PIN_MUX;
	config.mode_specific.master.baudrate = TEST_SPI_BAUDRATE;
	status = spi_init(&master, CONF_SPI_MASTER_MODULE, &config);
	test_assert_true(test, status == STATUS_OK,
			"SPI master initialization failed");

	/* Enable the SPI master */
	spi_enable(&master);

	status = STATUS_ERR_IO;
	/* Configure the SPI slave */
	spi_get_config_defaults(&config);
	config.mode                 = SPI_MODE_SLAVE;
	config.mux_setting          = CONF_SPI_SLAVE_SPI_MUX;
	config.pinmux_pad0          = CONF_SPI_SLAVE_DATA_IN_PIN_MUX;
	config.pinmux_pad1          = CONF_SPI_SLAVE_SS_PIN_MUX;
	config.pinmux_pad2          = CONF_SPI_SLAVE_DATA_OUT_PIN_MUX;
	config.pinmux_pad3          = CONF_SPI_SLAVE_SCK_PIN_MUX;
	config.mode_specific.slave.frame_format   = SPI_FRAME_FORMAT_SPI_FRAME;
	config.mode_specific.slave.preload_enable = true;
	status = spi_init(&slave, CONF_SPI_SLAVE_MODULE, &config);
	test_assert_true(test, status == STATUS_OK,
			"SPI slave initialization failed");

	/* Enable the SPI slave */
	spi_enable(&slave);
	if (status == STATUS_OK) {
		spi_init_success = true;
	}
}

/**
 * \internal
 * \brief Test sending and receiving a byte by polling.
 *
 * This test sends (writes) one byte of data to the slave and
 * receives (reads) the data back and compares.
 *
 * Writing and reading are carried out by polling.
 *
 * \param test Current test case.
 */
static void run_single_byte_polled_test(const struct test_case *test)
{
	uint16_t txd_data = 0x55;
	uint16_t rxd_data = 0;

	/* Skip test if initialization failed */
	test_assert_true(test, spi_init_success,
			"Skipping test due to failed initialization");

	/* Send data to slave */
	spi_select_slave(&master, &slave_inst, true);
	while (!spi_is_ready_to_write(&master)) {
	}
	spi_write(&master, txd_data);
	while (!spi_is_write_complete(&master)) {
	}

	/* Dummy read SPI master data register */
	while (!spi_is_ready_to_read(&master)) {
	}
	spi_read(&master, &rxd_data);

	/* Read SPI slave data register */
	while (!spi_is_ready_to_read(&slave)) {
	}
	spi_read(&slave, &rxd_data);
	spi_select_slave(&master, &slave_inst, false);

	/* Output test result */
	test_assert_true(test, rxd_data == txd_data,
			"Failed transmitting/receiving byte. TX='%d', RX='%d'",
			txd_data, rxd_data);
}

/**
 * \internal
 * \brief Setup function: Send data by polling & receive with interrupt.
 *
 * This function registers and enables callback for buffer receive
 * completed operation.
 *
 * \param test Current test case.
 */
static void setup_buffer_polled_write_interrupt_read_test
	(const struct test_case *test)
{
	/* Register & enable callback for buffer reception */
	spi_register_callback(&slave, user_spi_callback,
			SPI_CALLBACK_BUFFER_RECEIVED);
	spi_enable_callback(&slave, SPI_CALLBACK_BUFFER_RECEIVED);
}

/**
 * \internal
 * \brief Cleanup function: Send data by polling & receive with interrupt.
 *
 * This function unregisters and disables callback for buffer receive
 * completed operation. It also clears the receive buffer rx_buf.
 *
 * \param test Current test case.
 */
static void cleanup_buffer_polled_write_interrupt_read_test
	(const struct test_case *test)
{
	/* Unregister & disable callback */
	spi_unregister_callback(&slave, SPI_CALLBACK_BUFFER_RECEIVED);
	spi_disable_callback(&slave, SPI_CALLBACK_BUFFER_RECEIVED);

	/* Clear receive buffer for next test */
	for (uint16_t i = 0; i < BUFFER_LENGTH; i++) {
		rx_buf[i] = 0;
	}
}

/**
 * \internal
 * \brief Test: Send data by polling & receive with interrupt.
 *
 * This test sends (writes) an array of data to the slave by polling and
 * receives (reads) the buffer back with interrupt and compares.
 *
 * \param test Current test case.
 */
static void run_buffer_polled_write_interrupt_read_test
	(const struct test_case *test)
{
	uint16_t i;
	uint16_t timeout_cycles;

	/* Skip test if initialization failed */
	test_assert_true(test, spi_init_success,
			"Skipping test due to failed initialization");

	/* Start the test */
	transfer_complete = false;
	timeout_cycles = 1000;
	spi_select_slave(&master, &slave_inst, true);
	spi_read_buffer_job(&slave, rx_buf, BUFFER_LENGTH, 0);
	spi_write_buffer_wait(&master, tx_buf, BUFFER_LENGTH);

	/* Wait until reception completes */
	do {
		timeout_cycles--;
		if (transfer_complete) {
			break;
		}
	} while (timeout_cycles != 0);
	spi_select_slave(&master, &slave_inst, false);

	test_assert_true(test, timeout_cycles > 0,
			"Timeout in reception");

	/* Compare received data with transmitted data */
	if (transfer_complete) {
		for (i = 0; i < BUFFER_LENGTH; i++) {
			test_assert_true(test, tx_buf[i] == rx_buf[i],
					"Bytes differ at buffer index %d : %d != %d",
					i, tx_buf[i], rx_buf[i]);
		}
	}
}

/**
 * \internal
 * \brief Test: Send & receive data using transceive functions.
 *
 * This test sends (writes) an array of data to the slave and
 * receives (reads) the buffer back using transceive functions
 * and compares.
 *
 * \param test Current test case.
 */
static void run_transceive_buffer_test(const struct test_case *test)
{
	enum status_code status = STATUS_ERR_IO;

	/* Skip test if initialization failed */
	test_assert_true(test, spi_init_success,
			"Skipping test due to failed initialization");

	/* Start the test */
	spi_transceive_buffer_job(&slave, slave_tx_buf, slave_rx_buf,
			BUFFER_LENGTH);
	spi_select_slave(&master, &slave_inst, true);
	status = spi_transceive_buffer_wait(&master, tx_buf, rx_buf,
			BUFFER_LENGTH);
	spi_select_slave(&master, &slave_inst, false);

	test_assert_true(test, status == STATUS_OK,
			"Transceive buffer failed");

	/* Compare received data with transmitted data */
	if (status == STATUS_OK) {
		for (uint16_t i = 0; i < BUFFER_LENGTH; i++) {
			test_assert_true(test, tx_buf[i] == slave_rx_buf[i],
					"During TX: Bytes differ at buffer index %d : %d != %d",
					i, tx_buf[i], slave_rx_buf[i]);
			test_assert_true(test, slave_tx_buf[i] == rx_buf[i],
					"During RX: Bytes differ at buffer index %d : %d != %d",
					i, slave_tx_buf[i], rx_buf[i]);
		}
	}
}

/**
 * \internal
 * \brief Test: Sends data at different baud rates.
 *
 * This test sends (writes) a byte to the slave and receives the data
 * at different baudrate testing up to the maximum allowed level.
 *
 * Transmission and reception are carried out by polling.
 *
 * \param test Current test case.
 */
static void run_baud_test(const struct test_case *test)
{
	uint32_t test_baud = 1000000;
	uint8_t txd_data   = 0x55;
	uint8_t rxd_data   = 0;
	bool max_baud      = true;

	/* Skip test if initialization failed */
	test_assert_true(test, spi_init_success,
			"Skipping test due to failed initialization");

	/* Structure for SPI configuration */
	struct spi_config config;

	/* Configure the SPI master */
	spi_get_config_defaults(&config);
	config.mux_setting     = CONF_SPI_MASTER_SPI_MUX;
	config.pinmux_pad0     = CONF_SPI_MASTER_DATA_IN_PIN_MUX;
	config.pinmux_pad1     = PINMUX_UNUSED;
	config.pinmux_pad2     = CONF_SPI_MASTER_DATA_OUT_PIN_MUX;
	config.pinmux_pad3     = CONF_SPI_MASTER_SCK_PIN_MUX;

	do {
		spi_disable(&master);
		config.mode_specific.master.baudrate = test_baud;
		spi_init(&master, CONF_SPI_MASTER_MODULE, &config);
		spi_enable(&master);

		/* Send data to slave */
		spi_select_slave(&master, &slave_inst, true);
		spi_write_buffer_wait(&master, &txd_data, 1);
		spi_read_buffer_wait(&slave, &rxd_data, 1, 0);
		spi_select_slave(&master, &slave_inst, false);

		if (txd_data != rxd_data) {
			max_baud = false;
			break;
		}

		test_baud += 1000000;
	} while (test_baud <= 24000000);

	/* Output the result */
	test_assert_true(test, max_baud,
			"Test failed at baudrate: %lu", test_baud);
}

/**
 * \internal
 * \brief Setup function: Send & receive 9-bit data by polling.
 *
 * This function configures the SPI master & slave in 9-bit mode.
 *
 * \param test Current test case.
 */
static void setup_transfer_9bit_test(const struct test_case *test)
{
	enum status_code status = STATUS_ERR_IO;
	spi_init_success = false;

	/* Structure for SPI configuration */
	struct spi_config config;

	spi_disable(&master);
	spi_disable(&slave);

	/* Configure the SPI master */
	spi_get_config_defaults(&config);
	config.mux_setting     = CONF_SPI_MASTER_SPI_MUX;
	config.pinmux_pad0     = CONF_SPI_MASTER_DATA_IN_PIN_MUX;
	config.pinmux_pad1     = PINMUX_UNUSED;
	config.pinmux_pad2     = CONF_SPI_MASTER_DATA_OUT_PIN_MUX;
	config.pinmux_pad3     = CONF_SPI_MASTER_SCK_PIN_MUX;
	config.mode_specific.master.baudrate = TEST_SPI_BAUDRATE;
	config.character_size  = SPI_CHARACTER_SIZE_9BIT;
	status = spi_init(&master, CONF_SPI_MASTER_MODULE, &config);
	test_assert_true(test, status == STATUS_OK,
			"SPI master initialization failed for 9-bit configuration");
	/* Enable the SPI master */
	spi_enable(&master);

	status = STATUS_ERR_IO;

	/* Configure the SPI slave */
	spi_get_config_defaults(&config);
	config.mode                 = SPI_MODE_SLAVE;
	config.mux_setting          = CONF_SPI_SLAVE_SPI_MUX;
	config.pinmux_pad0          = CONF_SPI_SLAVE_DATA_IN_PIN_MUX;
	config.pinmux_pad1          = CONF_SPI_SLAVE_SS_PIN_MUX;
	config.pinmux_pad2          = CONF_SPI_SLAVE_DATA_OUT_PIN_MUX;
	config.pinmux_pad3          = CONF_SPI_SLAVE_SCK_PIN_MUX;
	config.mode_specific.slave.frame_format   = SPI_FRAME_FORMAT_SPI_FRAME;
	config.mode_specific.slave.preload_enable = true;
	config.character_size       = SPI_CHARACTER_SIZE_9BIT;
	status = spi_init(&slave, CONF_SPI_SLAVE_MODULE, &config);
	test_assert_true(test, status == STATUS_OK,
			"SPI slave initialization failed for 9-bit configuration");

	/* Enable the SPI slave */
	spi_enable(&slave);
	if (status == STATUS_OK) {
		spi_init_success = true;
	}
}

/**
 * \internal
 * \brief Test sending and receiving 9-bit data by polling.
 *
 * This test sends (writes) one 9-bit data to the slave and
 * receives (reads) the data back and compares.
 *
 * Writing and reading are carried out by polling.
 *
 * \param test Current test case.
 */
static void run_transfer_9bit_test(const struct test_case *test)
{
	uint16_t txd_data = 0x155;
	uint16_t rxd_data = 0;

	/* Skip test if initialization failed */
	test_assert_true(test, spi_init_success,
			"Skipping test due to failed initialization");

	/* Send data to slave */
	spi_select_slave(&master, &slave_inst, true);
	while (!spi_is_ready_to_write(&master)) {
	}
	spi_write(&master, txd_data);
	while (!spi_is_write_complete(&master)) {
	}

	/* Dummy read SPI master data register */
	while (!spi_is_ready_to_read(&master)) {
	}
	spi_read(&master, &rxd_data);

	/* Read SPI slave data register */
	while (!spi_is_ready_to_read(&slave)) {
	}
	spi_read(&slave, &rxd_data);
	spi_select_slave(&master, &slave_inst, false);

	/* Output test result */
	test_assert_true(test, rxd_data == txd_data,
			"Failed transmitting/receiving byte. TX='%d', RX='%d'",
			txd_data, rxd_data);
}

/**
 * \brief Run SPI unit tests
 *
 * Initializes the system and serial output, then sets up the
 * SPI unit test suite and runs it.
 */
int main(void)
{
	system_init();
	cdc_uart_init();
	cpu_irq_enable();

	/* Fill the transmit buffers with some data */
	for (uint16_t i = 0; i < BUFFER_LENGTH; i++) {
		tx_buf[i] = i + 1;
		slave_tx_buf[i] = i + 1;
	}

	/* Define Test Cases */
	DEFINE_TEST_CASE(spi_init_test, NULL,
			run_spi_init_test, NULL,
			"Initialization test for SPI master & slave");

	DEFINE_TEST_CASE(single_byte_polled_test, NULL,
			run_single_byte_polled_test, NULL,
			"Transfer single byte and readback by polling");

	DEFINE_TEST_CASE(buffer_polled_write_interrupt_read_test,
			setup_buffer_polled_write_interrupt_read_test,
			run_buffer_polled_write_interrupt_read_test,
			cleanup_buffer_polled_write_interrupt_read_test,
			"Transfer bytes by polling and read back with interrupt");

	DEFINE_TEST_CASE(transceive_buffer_test, NULL,
			run_transceive_buffer_test, NULL,
			"Transmit & receive bytes using transceive functions");

	DEFINE_TEST_CASE(baud_test, NULL, run_baud_test, NULL,
			"Transfer byte at different baud rates");

	DEFINE_TEST_CASE(transfer_9bit_test, setup_transfer_9bit_test,
			run_transfer_9bit_test, NULL,
			"Transfer 9-bit character and readback by polling");

	/* Put test case addresses in an array */
	DEFINE_TEST_ARRAY(spi_tests) = {
		&spi_init_test,
		&single_byte_polled_test,
		&buffer_polled_write_interrupt_read_test,
		&transceive_buffer_test,
		&baud_test,
		&transfer_9bit_test,
	};

	/* Define the test suite */
	DEFINE_TEST_SUITE(spi_test_suite, spi_tests,
			"SAM D20/D21/R21 SPI driver test suite");

	/* Run all tests in the suite*/
	test_suite_run(&spi_test_suite);

	while (true) {
		/* Intentionally left empty */
	}
}
