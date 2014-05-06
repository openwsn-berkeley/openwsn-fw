/**
 * \file
 *
 * \brief SAM D21/R21 Sercom SPI driver with DMA quick start
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
#include "conf_quick_start.h"

//! [buf_length]
#define BUF_LENGTH 20
//! [buf_length]

//! [spi_baudrate]
#define TEST_SPI_BAUDRATE             1000000UL
//! [spi_baudrate]

//! [slave_select_pin]
#define SLAVE_SELECT_PIN CONF_MASTER_SS_PIN
//! [slave_select_pin]

//! [spi_buffer]
static const uint8_t buffer_tx[BUF_LENGTH] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
		0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
};
static uint8_t buffer_rx[BUF_LENGTH];
//! [spi_buffer]

//! [spi_module_inst]
struct spi_module spi_master_instance;
struct spi_module spi_slave_instance;
//! [spi_module_inst]

//! [slave_dev_inst]
struct spi_slave_inst slave;
//! [slave_dev_inst]

//! [dma_resource]
struct dma_resource example_resource_tx;
struct dma_resource example_resource_rx;
//! [dma_resource]

//! [dma_transfer_done_flag]
static volatile bool transfer_tx_is_done = false;
static volatile bool transfer_rx_is_done = false;
//! [dma_transfer_done_flag]

//! [dma_transfer_descriptor]
COMPILER_ALIGNED(16)
DmacDescriptor example_descriptor_tx;
DmacDescriptor example_descriptor_rx;
//! [dma_transfer_descriptor]

//! [setup]
//! [_transfer_tx_done]
static void transfer_tx_done( const struct dma_resource* const resource )
{
	transfer_tx_is_done = true;
}
//! [_transfer_tx_done]

//! [_transfer_rx_done]
static void transfer_rx_done( const struct dma_resource* const resource )
{
	transfer_rx_is_done = true;
}
//! [_transfer_rx_done]

//! [config_dma_resource_tx]
static void configure_dma_resource_tx(struct dma_resource *tx_resource)
{
//! [dma_tx_setup_1]
	struct dma_resource_config tx_config;
//! [dma_tx_setup_1]

//! [dma_tx_setup_2]
	dma_get_config_defaults(&tx_config);
//! [dma_tx_setup_2]

//! [dma_tx_setup_3]
	tx_config.transfer_trigger = DMA_TRIGGER_PERIPHERAL;
	tx_config.peripheral_trigger = CONF_PERIPHERAL_TRIGGER_TX;
	tx_config.trigger_action = DMA_TRIGGER_ACTON_BEAT;
//! [dma_tx_setup_3]

//! [dma_tx_setup_4]
	dma_allocate(tx_resource, &tx_config);
//! [dma_tx_setup_4]
}
//! [config_dma_resource_tx]

//! [config_dma_resource_rx]
static void configure_dma_resource_rx(struct dma_resource *rx_resource)
{
//! [dma_rx_setup_1]
	struct dma_resource_config rx_config;
//! [dma_rx_setup_1]

//! [dma_rx_setup_2]
	dma_get_config_defaults(&rx_config);
//! [dma_rx_setup_2]

//! [dma_rx_setup_3]
	rx_config.transfer_trigger = DMA_TRIGGER_PERIPHERAL;
	rx_config.peripheral_trigger = CONF_PERIPHERAL_TRIGGER_RX;
	rx_config.trigger_action = DMA_TRIGGER_ACTON_BEAT;
//! [dma_rx_setup_3]

//! [dma_rx_setup_4]
	dma_allocate(rx_resource, &rx_config);
//! [dma_rx_setup_4]
}
//! [config_dma_resource_rx]

//! [setup_dma_transfer_descriptor]
static void setup_transfer_descriptor_tx(DmacDescriptor *tx_descriptor)
{
//! [dma_tx_setup_5]
	struct dma_descriptor_config tx_descriptor_config;
//! [dma_tx_setup_5]

//! [dma_tx_setup_6]
	dma_descriptor_get_config_defaults(&tx_descriptor_config);
//! [dma_tx_setup_6]

//! [dma_tx_setup_7]
	tx_descriptor_config.beat_size = DMA_BEAT_SIZE_BYTE;
	tx_descriptor_config.dst_increment_enable = false;
	tx_descriptor_config.block_transfer_count = sizeof(buffer_tx)/sizeof(uint8_t);
	tx_descriptor_config.source_address = (uint32_t)buffer_tx + sizeof(buffer_tx);
	tx_descriptor_config.destination_address =
		(uint32_t)(&spi_master_instance.hw->SPI.DATA.reg);
//! [dma_tx_setup_7]

//! [dma_tx_setup_8]
	dma_descriptor_create(tx_descriptor, &tx_descriptor_config);
//! [dma_tx_setup_8]
}
//! [setup_dma_transfer_descriptor]

//! [setup_dma_transfer_descriptor_rx]
static void setup_transfer_descriptor_rx(DmacDescriptor *rx_descriptor)
{
//! [dma_rx_setup_5]
	struct dma_descriptor_config rx_descriptor_config;
//! [dma_rx_setup_5]

//! [dma_rx_setup_6]
	dma_descriptor_get_config_defaults(&rx_descriptor_config);
//! [dma_rx_setup_6]

//! [dma_rx_setup_7]
	rx_descriptor_config.beat_size = DMA_BEAT_SIZE_BYTE;
	rx_descriptor_config.src_increment_enable = false;
	rx_descriptor_config.block_transfer_count = sizeof(buffer_rx)/sizeof(uint8_t);
	rx_descriptor_config.source_address =
		(uint32_t)(&spi_slave_instance.hw->SPI.DATA.reg);
	rx_descriptor_config.destination_address =
		(uint32_t)buffer_rx + sizeof(buffer_rx);
//! [dma_rx_setup_7]

//! [dma_rx_setup_8]
	dma_descriptor_create(rx_descriptor, &rx_descriptor_config);
//! [dma_rx_setup_8]
}
//! [setup_dma_transfer_descriptor_rx]

//! [configure_spi]
static void configure_spi_master(void)
{
//! [spi_master_config]
	struct spi_config config_spi_master;
//! [spi_master_config]
//! [slave_config]
	struct spi_slave_inst_config slave_dev_config;
//! [slave_config]
	/* Configure and initialize software device instance of peripheral slave */
//! [slave_conf_defaults]
	spi_slave_inst_get_config_defaults(&slave_dev_config);
//! [slave_conf_defaults]
//! [ss_pin]
	slave_dev_config.ss_pin = SLAVE_SELECT_PIN;
//! [ss_pin]
//! [slave_init]
	spi_attach_slave(&slave, &slave_dev_config);
//! [slave_init]
	/* Configure, initialize and enable SERCOM SPI module */
//! [spi_master_conf_defaults]
	spi_get_config_defaults(&config_spi_master);
//! [spi_master_conf_defaults]
	config_spi_master.mode_specific.master.baudrate = TEST_SPI_BAUDRATE;
//! [spi_master_mux_setting]
	config_spi_master.mux_setting = CONF_MASTER_MUX_SETTING;
//! [spi_master_mux_setting]
	/* Configure pad 0 for data in */
//! [di]
	config_spi_master.pinmux_pad0 = CONF_MASTER_PINMUX_PAD0;
//! [di]
	/* Configure pad 1 as unused */
//! [ss]
	config_spi_master.pinmux_pad1 = CONF_MASTER_PINMUX_PAD1;
//! [ss]
	/* Configure pad 2 for data out */
//! [do]
	config_spi_master.pinmux_pad2 = CONF_MASTER_PINMUX_PAD2;
//! [do]
	/* Configure pad 3 for SCK */
//! [sck]
	config_spi_master.pinmux_pad3 = CONF_MASTER_PINMUX_PAD3;
//! [sck]
//! [spi_master_init]
	spi_init(&spi_master_instance, CONF_MASTER_SPI_MODULE, &config_spi_master);
//! [spi_master_init]

//! [spi_master_enable]
	spi_enable(&spi_master_instance);
//! [spi_master_enable]

}
//! [configure_spi]

//! [configure_spi_slave]
static void configure_spi_slave(void)
{
//! [spi_slave_config]
	struct spi_config config_spi_slave;
//! [spi_slave_config]

	/* Configure, initialize and enable SERCOM SPI module */
//! [spi_slave_conf_defaults]
	spi_get_config_defaults(&config_spi_slave);
//! [spi_slave_conf_defaults]
//! [conf_spi_slave_instance]
	config_spi_slave.mode = SPI_MODE_SLAVE;
//! [conf_spi_slave_instance]
//! [conf_preload]
	config_spi_slave.mode_specific.slave.preload_enable = true;
//! [conf_preload]
//! [conf_format]
	config_spi_slave.mode_specific.slave.frame_format = SPI_FRAME_FORMAT_SPI_FRAME;
//! [conf_format]
//! [spi_slave_mux_setting]
	config_spi_slave.mux_setting = CONF_SLAVE_MUX_SETTING;
//! [spi_slave_mux_setting]
	/* Configure pad 0 for data in */
//! [di]
	config_spi_slave.pinmux_pad0 = CONF_SLAVE_PINMUX_PAD0;
//! [di]
	/* Configure pad 1 as unused */
//! [ss]
	config_spi_slave.pinmux_pad1 = CONF_SLAVE_PINMUX_PAD1;
//! [ss]
	/* Configure pad 2 for data out */
//! [do]
	config_spi_slave.pinmux_pad2 = CONF_SLAVE_PINMUX_PAD2;
//! [do]
	/* Configure pad 3 for SCK */
//! [sck]
	config_spi_slave.pinmux_pad3 = CONF_SLAVE_PINMUX_PAD3;
//! [sck]
//! [spi_slave_init]
	spi_init(&spi_slave_instance, CONF_SLAVE_SPI_MODULE, &config_spi_slave);
//! [spi_slave_init]

//! [spi_slave_enable]
	spi_enable(&spi_slave_instance);
//! [spi_slave_enable]

}
//! [configure_spi_slave]
//! [setup]

int main(void)
{
	system_init();

//! [setup_init]
//! [setup_spi]
	configure_spi_master();
	configure_spi_slave();
//! [setup_spi]

//! [setup_dma_resource]
	configure_dma_resource_tx(&example_resource_tx);
	configure_dma_resource_rx(&example_resource_rx);
//! [setup_dma_resource]

//! [setup_transfer_descriptor]
	setup_transfer_descriptor_tx(&example_descriptor_tx);
	setup_transfer_descriptor_rx(&example_descriptor_rx);
//! [setup_transfer_descriptor]

//! [dma_resource_add_descriptor]
	dma_add_descriptor(&example_resource_tx, &example_descriptor_tx);
	dma_add_descriptor(&example_resource_rx, &example_descriptor_rx);
//! [dma_resource_add_descriptor]

//! [setup_callback_register]
	dma_register_callback(&example_resource_tx, transfer_tx_done,
			DMA_CALLBACK_TRANSFER_DONE);
	dma_register_callback(&example_resource_rx, transfer_rx_done,
			DMA_CALLBACK_TRANSFER_DONE);
//! [setup_callback_register]

//! [setup_enable_callback]
	dma_enable_callback(&example_resource_tx, DMA_CALLBACK_TRANSFER_DONE);
	dma_enable_callback(&example_resource_rx, DMA_CALLBACK_TRANSFER_DONE);
//! [setup_enable_callback]
//! [setup_init]

//! [main]

//! [select_slave]
	spi_select_slave(&spi_master_instance, &slave, true);
//! [select_slave]

//! [main_1]
	dma_start_transfer_job(&example_resource_rx);
	dma_start_transfer_job(&example_resource_tx);
//! [main_1]

//! [main_2]
	while (!transfer_rx_is_done) {
		/* Wait for transfer done */
	}
//! [main_2]

//! [deselect_slave]
	spi_select_slave(&spi_master_instance, &slave, false);
//! [deselect_slave]

//! [endless_loop]
	while (true) {
	}
//! [endless_loop]

//! [main]
}
