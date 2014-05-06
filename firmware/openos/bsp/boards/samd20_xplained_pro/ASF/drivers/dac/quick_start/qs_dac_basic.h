/**
 * \file
 *
 * \brief SAM D20/D21 DAC Quick Start
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
 * \page asfdoc_sam0_dac_basic_use_case Quick Start Guide for DAC - Basic
 *
 * In this use case, the DAC will be configured with the following settings:
 * - Analog VCC as reference
 * - Internal output disabled
 * - Drive the DAC output to the V<sub>OUT</sub> pin
 * - Right adjust data
 * - The output buffer is disabled when the chip enters STANDBY sleep mode
 *
 * \section asfdoc_sam0_dac_basic_use_case_setup Quick Start
 *
 * \subsection asfdoc_sam0_dac_basic_use_case_prereq Prerequisites
 * There are no special setup requirements for this use-case.
 *
 * \subsection asfdoc_sam0_dac_basic_use_case_setup_code Code
 * Add to the main application source file, outside of any functions:
 * \snippet qs_dac_basic.c module_inst
 *
 * Copy-paste the following setup code to your user application:
 * \snippet qs_dac_basic.c setup
 *
 * Add to user application initialization (typically the start of \c main()):
 * \snippet qs_dac_basic.c setup_init
 *
 * \subsection asfdoc_sam0_dac_basic_use_case_setup_flow Workflow
 * -# Create a module software instance structure for the DAC module to store
 *    the DAC driver state while it is in use.
 *    \snippet qs_dac_basic.c module_inst
 *    \note This should never go out of scope as long as the module is in use.
 *          In most cases, this should be global.
 *
 * -# Configure the DAC module.
 *  -# Create a DAC module configuration struct, which can be filled out to
 *     adjust the configuration of a physical DAC peripheral.
 *     \snippet qs_dac_basic.c setup_config
 *  -# Initialize the DAC configuration struct with the module's default values.
 *     \snippet qs_dac_basic.c setup_config_defaults
 *     \note This should always be performed before using the configuration
 *           struct to ensure that all values are initialized to known default
 *           settings.
 *
 *  -# Enable the DAC module so that channels can be configured.
 *     \snippet qs_dac_basic.c setup_enable
 * -# Configure the DAC channel.
 *  -# Create a DAC channel configuration struct, which can be filled out to
 *     adjust the configuration of a physical DAC output channel.
 *     \snippet qs_dac_basic.c setup_ch_config
 *  -# Initialize the DAC channel configuration struct with the module's default
 *     values.
 *     \snippet qs_dac_basic.c setup_ch_config_defaults
 *     \note This should always be performed before using the configuration
 *           struct to ensure that all values are initialized to known default
 *           settings.
 *
 *  -# Configure the DAC channel with the desired channel settings.
 *     \snippet qs_dac_basic.c setup_ch_set_config
 *  -# Enable the DAC channel so that it can output a voltage.
 *     \snippet qs_dac_basic.c setup_ch_enable
 *
 * \section asfdoc_sam0_dac_basic_use_case_main Use Case
 *
 * \subsection asfdoc_sam0_dac_basic_use_case_main_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_dac_basic.c main
 *
 * \subsection asfdoc_sam0_dac_basic_use_case_main_flow Workflow
 * -# Create a temporary variable to track the current DAC output value.
 *    \snippet qs_dac_basic.c main_output_var
 * -# Enter an infinite loop to continuously output new conversion values to
 *    the DAC.
 *    \snippet qs_dac_basic.c main_loop
 * -# Write the next conversion value to the DAC, so that it will be output on
 *    the device's DAC analog output pin.
 *    \snippet qs_dac_basic.c main_write
 * -# Increment and wrap the DAC output conversion value, so that a ramp pattern
 *    will be generated.
 *    \snippet qs_dac_basic.c main_inc_val
 */
