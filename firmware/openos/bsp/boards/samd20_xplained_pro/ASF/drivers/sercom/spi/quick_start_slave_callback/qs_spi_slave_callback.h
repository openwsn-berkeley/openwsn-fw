/**
 * \file
 *
 * \brief SAM D20/D21/R21 SPI Quick Start
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

/**
 * \page asfdoc_sam0_sercom_spi_slave_callback_use Quick Start Guide for SERCOM SPI Slave - Callback
 *
 * In this use case, the SPI on extension header 1 of the Xplained Pro board
 * will configured with the following settings:
 * - Slave mode enabled
 * - Preloading of shift register enabled
 * - MSB of the data is transmitted first
 * - Transfer mode 0
 * - SPI MUX Setting E (see \ref asfdoc_sam0_sercom_spi_mux_settings_slave)
 *   - MISO on pad 2, extension header 1, pin 16
 *   - MOSI on pad 0, extension header 1, pin 17
 *   - SCK on pad 3, extension header 1, pin 18
 *   - SS on pad 1, extension header 1, pin 15
 * - 8-bit character size
 * - Not enabled in sleep mode
 * - GLCK generator 0
 *
 *
 * \section asfdoc_sam0_sercom_spi_slave_callback_use_setup Setup
 *
 * \subsection asfdoc_sam0_sercom_spi_slave_callback_useprereq Prerequisites
 * The device must be connected to a SPI master which must read from the device.
 *
 * \subsection asfdoc_sam0_sercom_spi_slave_callback_use_setup_code Code
 * The following must be added to the user application source file, outside
 * any functions:
 *
 * A sample buffer to send via SPI:
 * \snippet qs_spi_slave_callback.c buffer
 * Number of entries in the sample buffer:
 * \snippet qs_spi_slave_callback.c buf_length
 * A globally available software device instance struct to store the SPI driver
 * state while it is in use.
 * \snippet qs_spi_slave_callback.c dev_inst
 * A function for configuring the SPI:
 * \snippet qs_spi_slave_callback.c configure_spi
 * A function for configuring the callback functionality of the SPI:
 * \snippet qs_spi_slave_callback.c conf_callback
 * A global variable that can flag to the application that the buffer has been
 * transferred:
 * \snippet qs_spi_slave_callback.c var
 * Callback function:
 * \snippet qs_spi_slave_callback.c callback
 *
 * Add to user application %main():
 * \snippet qs_spi_slave_callback.c main_start
 *
 * \subsection asfdoc_sam0_sercom_spi_slave_callback_use_workflow Workflow
 * -# Initialize system.
 *    \snippet qs_spi_slave_callback.c system_init
 * -# Setup the SPI:
 *    \snippet qs_spi_slave_callback.c run_config
 *   -# Create configuration struct.
 *      \snippet qs_spi_slave_callback.c config
 *   -# Get default configuration to edit.
 *      \snippet qs_spi_slave_callback.c conf_defaults
 *   -# Set the SPI in slave mode.
 *      \snippet qs_spi_slave_callback.c conf_spi_slave_instance
 *   -# Enable preloading of shift register.
 *      \snippet qs_spi_slave_callback.c conf_preload
 *   -# Set frame format to SPI frame.
 *      \snippet qs_spi_slave_callback.c conf_format
 *   -# Set mux setting E.
 *      \snippet qs_spi_slave_callback.c mux_setting
 *   -# Set pinmux for pad 0 (data in (MOSI) on extension header 1, pin 17).
 *      \snippet qs_spi_slave_callback.c di
 *   -# Set pinmux for pad 1 (slave select on on extension header 1, pin 15)
 *      \snippet qs_spi_slave_callback.c ss
 *   -# Set pinmux for pad 2 (data out (MISO) on extension header 1, pin 16).
 *      \snippet qs_spi_slave_callback.c do
 *   -# Set pinmux for pad 3 (SCK on extension header 1, pin 18).
 *      \snippet qs_spi_slave_callback.c sck
 *   -# Initialize SPI module with configuration.
 *      \snippet qs_spi_slave_callback.c init
 *   -# Enable SPI module.
 *      \snippet qs_spi_slave_callback.c enable
 * -# Setup the callback functionality:
 *    \snippet qs_spi_slave_callback.c run_callback_config
 *   -# Register callback function for buffer transmitted
 *      \snippet qs_spi_slave_callback.c reg_callback
 *   -# Enable callback for buffer transmitted
 *      \snippet qs_spi_slave_callback.c en_callback
 *
 * \section asfdoc_sam0_sercom_spi_slave_callback_usecase Use Case
 * \subsection asfdoc_sam0_sercom_spi_slave_callback_usecase_code Code
 * Add the following to your user application \c main():
 * \snippet qs_spi_slave_callback.c main_use_case
 * \subsection asfdoc_sam0_sercom_spi_slave_callback_usecase_workflow Workflow
 * -# Initiate a write buffer job.
 *    \snippet qs_spi_slave_callback.c write
 * -# Wait for the transfer to be complete.
 *    \snippet qs_spi_slave_callback.c transf_complete
 * -# Infinite loop.
 *    \snippet qs_spi_slave_callback.c inf_loop
 *
 * \section asfdoc_sam0_sercom_spi_slave_callback_use_callback Callback
 * When the buffer is successfully transmitted to the master, the callback
 * function will be called.
 * \subsection asfdoc_sam0_sercom_spi_slave_callback_use_callback_workflow Workflow
 * -# Let the application know that the buffer is transmitted by setting the
 *    global variable to true.
 *    \snippet qs_spi_slave_callback.c callback_var
 */
