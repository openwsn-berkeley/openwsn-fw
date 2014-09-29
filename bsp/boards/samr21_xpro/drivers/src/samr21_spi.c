/**
* Copyright (c) 2014 Atmel Corporation. All rights reserved. 
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* 
* 3. The name of Atmel may not be used to endorse or promote products derived 
* from this software without specific prior written permission.  
* 
* 4. This software may only be redistributed and used in connection with an 
* Atmel microcontroller product.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* 
*/

#include <sam.h>
#include "samr21_spi.h"
#include "samr21_gclk.h"
#include "samr21_gpio.h"
#include "samr21_sysctrl.h"


/** \brief spi_init This function will initialize the SPI Connection

     Configuring the SPI in master mode and also enables the slave pin for communication.

    \param [in] None
    \return None
 */
void sercom_spi_init(void)
{
	Sercom *const hw = SPI_MODULE;
	pinmux_t config;
	
	/* Initialize device instance */
	SercomSpi *const spi_module = &(hw->SPI);

	/* Check if reset is in progress. */
	while(spi_module->CTRLA.reg & SERCOM_SPI_CTRLA_SWRST){
		
	}
	/* Turn on module in PM */
	PM->APBCMASK.reg |= SPI_PM_MODULE;
	
	/* Set up the GCLK for the module */
	gclk_chan_config(SPI_MODULE_GCLK, SPI_GCLK);
	gclk_enable(SPI_MODULE_GCLK);
	gclk_chan_config(SERCOM_GCLK_ID, SPI_GCLK);
	gclk_enable(SERCOM_GCLK_ID);
	
	/* Wait for synchronization to complete */
	while(spi_module->SYNCBUSY.reg);
	/* Set the SERCOM in SPI master mode */
	spi_module->CTRLA.reg |= SERCOM_SPI_CTRLA_MODE_SPI_MASTER;
	
	/* Wait for synchronization to complete */
	while(spi_module->SYNCBUSY.reg);

	/* Configure the SERCOM pins according to the user configuration */	
	config.dir = PORT_PIN_DIR_INPUT;
	config.pull =  PORT_PIN_PULLUP;
	config.mux_loc = SPI_MISO_MUX & 0xFFFF;
	pinmux_config(SPI_MISO_PIN, &config);
	config.mux_loc = SPI_MOSI_MUX & 0xFFFF;
	pinmux_config(SPI_MOSI_PIN, &config);
	config.mux_loc = SPI_SCK_MUX & 0xFFFF;
	pinmux_config(SPI_SCK_PIN, &config);
	
	/* Wait for synchronization to complete */
	while(spi_module->SYNCBUSY.reg);
	spi_module->BAUD.reg = (uint8_t)SPI_BAUDRATE;
	
	/* Wait for synchronization to complete */
	while(spi_module->SYNCBUSY.reg);	
	/* Write CTRLA register */
	spi_module->CTRLA.reg |= (SPI_TRANSFER_MODE_0 | SPI_SERCOM_MUX_SETTING);

	/* Wait for synchronization to complete */
	while(spi_module->SYNCBUSY.reg);	
	/* Write CTRLB register */
	spi_module->CTRLB.reg |= (SERCOM_SPI_CTRLB_CHSIZE(0) | SERCOM_SPI_CTRLB_RXEN);
	
	/* Wait for synchronization to complete */
	while(spi_module->SYNCBUSY.reg);	
	/* Enable SPI */
	spi_module->CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE;
	
}

/** \brief spi_txrx_data This function will send one byte of data and receives one byte over SPI

     Data Input on MISO and Data Output on MOSI

    \param [in] *txrx_data 
    \return None
 */
void spi_txrx_data(uint8_t *txrx_data)
{
 uint16_t spi_data = (uint16_t)*txrx_data; 
 /* Check SPI is ready to write */
 Sercom *const hw = SPI_MODULE;
 SercomSpi *const spi_module = &(hw->SPI);

 /* Check DRE interrupt flag */
 while(!(spi_module->INTFLAG.reg & SERCOM_SPI_INTFLAG_DRE));
 /* Wait for the DRE */ 

 /* Write the character to the DATA register */
 spi_module->DATA.reg = spi_data & SERCOM_SPI_DATA_MASK;
 
 /* Check TXC interrupt flag */
  while(!(spi_module->INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC)){
	  /* Wait for the DRE */
  }
 
 /* Check RXC interrupt flag */
 while(!(spi_module->INTFLAG.reg & SERCOM_SPI_INTFLAG_RXC)){
 /* Wait for the DRE */
 }
 
 /* spi read data */
 spi_data = (spi_module->DATA.reg & SERCOM_SPI_DATA_MASK);
 *txrx_data = (uint8_t)spi_data;
}