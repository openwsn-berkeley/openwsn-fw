/**
 * \file
 *
 * \brief SAM SERCOM I2C Slave with DMA Quick Start Guide
 *
 * Copyright (C) 2014 Atmel Corporation. All rights reserved.
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

void configure_i2c_slave(void);
void setup_dma_descriptor(DmacDescriptor *descriptor);
void configure_dma_resource(struct dma_resource *resource);

//! [address]
#define SLAVE_ADDRESS 0x12
//! [address]

//! [packet_data]
#define DATA_LENGTH 10
uint8_t read_buffer[DATA_LENGTH];
//! [packet_data]

//! [module]
struct i2c_slave_module i2c_slave_instance;
//! [module]

//! [initialize_i2c]
void configure_i2c_slave(void)
{
	/* Create and initialize config_i2c_slave structure */
	//! [init_conf]
	struct i2c_slave_config config_i2c_slave;
	i2c_slave_get_config_defaults(&config_i2c_slave);
	//! [init_conf]

	/* Change address and address_mode */
	//! [conf_changes]
	config_i2c_slave.address        = SLAVE_ADDRESS;
	config_i2c_slave.address_mode   = I2C_SLAVE_ADDRESS_MODE_MASK;
	config_i2c_slave.buffer_timeout = 1000;
	//! [conf_changes]

	/* Initialize and enable device with config_i2c_slave */
	//! [init_module]
	i2c_slave_init(&i2c_slave_instance, SERCOM2, &config_i2c_slave);
	//! [init_module]

	//! [enable_module]
	i2c_slave_enable(&i2c_slave_instance);
	//! [enable_module]
}
//! [initialize_i2c]

//! [dma_resource]
struct dma_resource i2c_dma_resource;
//! [dma_resource]

// [transfer_descriptor]
COMPILER_ALIGNED(16)
DmacDescriptor i2c_dma_descriptor;
// [transfer_descriptor]

// [config_dma_resource]
void configure_dma_resource(struct dma_resource *resource)
{
	//! [dma_setup_1]
	struct dma_resource_config config;
	//! [dma_setup_1]

	//! [dma_setup_2]
	dma_get_config_defaults(&config);
	//! [dma_setup_2]

	//! [dma_setup_3]
	config.peripheral_trigger = SERCOM2_DMAC_ID_RX;
	config.trigger_action = DMA_TRIGGER_ACTON_BEAT;
	//! [dma_setup_3]

	//! [dma_setup_4]
	dma_allocate(resource, &config);
	//! [dma_setup_4]
}
// [config_dma_resource]

// [setup_dma_transfer_descriptor]
void setup_dma_descriptor(DmacDescriptor *descriptor)
{
	//! [dma_setup_5]
	struct dma_descriptor_config descriptor_config;
	//! [dma_setup_5]

	//! [dma_setup_6]
	dma_descriptor_get_config_defaults(&descriptor_config);
	//! [dma_setup_6]

	//! [dma_setup_7]
	descriptor_config.beat_size = DMA_BEAT_SIZE_BYTE;
	descriptor_config.src_increment_enable = false;
	descriptor_config.block_transfer_count = DATA_LENGTH;
	descriptor_config.destination_address = (uint32_t)read_buffer + DATA_LENGTH;
	descriptor_config.source_address = (uint32_t)(&i2c_slave_instance.hw->I2CS.DATA.reg);
	//! [dma_setup_7]

	//! [dma_setup_8]
	dma_descriptor_create(descriptor, &descriptor_config);
	//! [dma_setup_8]
}
// [setup_dma_transfer_descriptor]

int main(void)
{
	system_init();

	//! [init]
	//! [config_i2c]
	configure_i2c_slave();
	//! [config_i2c]

	//! [config_dma]
	configure_dma_resource(&i2c_dma_resource);
	setup_dma_descriptor(&i2c_dma_descriptor);
	dma_add_descriptor(&i2c_dma_resource, &i2c_dma_descriptor);
	//! [config_dma]
	//! [init]

	//! [main]
	//! [wait_packet]
	dma_start_transfer_job(&i2c_dma_resource);
	//! [wait_packet]

	//! [clear_status]
	while (true) {
		if (i2c_slave_dma_read_interrupt_status(&i2c_slave_instance) &
					SERCOM_I2CS_INTFLAG_AMATCH) {
			i2c_slave_dma_write_interrupt_status(&i2c_slave_instance,
					SERCOM_I2CS_INTFLAG_AMATCH);
		}
	}
	//! [clear_status]
	//! [main]
}
