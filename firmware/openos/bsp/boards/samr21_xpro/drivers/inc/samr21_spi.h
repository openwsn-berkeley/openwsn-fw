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


#ifndef samr21_spi_h__
#define samr21_spi_h__

#include "sam.h"

#define F_SPI (1024ul * 32768ul)
#define F_1MHz (1000000ul)

/* SPI baudrate calculation */
typedef enum
{
	SPI_BAUDRATE_2MHz  = ((F_SPI / (2 * 2 * F_1MHz)) - 1),
	SPI_BAUDRATE_4MHz  = ((F_SPI / (2 * 4 * F_1MHz)) - 1),
	SPI_BAUDRATE_8MHz  = ((F_SPI / (2 * 8 * F_1MHz)) - 1),
	SPI_BAUDRATE_10MHz = ((F_SPI / (2 * 10 * F_1MHz)) - 1)
} spi_baudrate_t;

/* Start of SPI Configuration */
#define SPI_MODULE              SERCOM4
#define SPI_GCLK				GCLK_GENERATOR_0
#define SPI_MODULE_GCLK			SERCOM4_GCLK_ID_CORE
#define SPI_PM_MODULE			PM_APBCMASK_SERCOM4
/** SPI MUX setting E */
#define SPI_SERCOM_MUX_SETTING  ((0x1 << SERCOM_SPI_CTRLA_DOPO_Pos) | (0x0 << SERCOM_SPI_CTRLA_DIPO_Pos))
#define SPI_MISO_MUX			MUX_PC19F_SERCOM4_PAD0
#define SPI_MOSI_MUX			MUX_PB30F_SERCOM4_PAD2
#define SPI_SCK_MUX				MUX_PC18F_SERCOM4_PAD3
#define SPI_MISO_PIN			PIN_PC19
#define SPI_MOSI_PIN			PIN_PB30
#define SPI_SCK_PIN				PIN_PC18
#define SPI_BAUDRATE			SPI_BAUDRATE_4MHz
/* End of SPI Configuration */

/**
 * \brief SPI transfer modes enum
 *
 * SPI transfer mode.
 */
enum spi_transfer_mode {
	/** Mode 0. Leading edge: rising, sample. Trailing edge: falling, setup */
	SPI_TRANSFER_MODE_0 = 0,
	/** Mode 1. Leading edge: rising, setup. Trailing edge: falling, sample */
	SPI_TRANSFER_MODE_1 = SERCOM_SPI_CTRLA_CPHA,
	/** Mode 2. Leading edge: falling, sample. Trailing edge: rising, setup */
	SPI_TRANSFER_MODE_2 = SERCOM_SPI_CTRLA_CPOL,
	/** Mode 3. Leading edge: falling, setup. Trailing edge: rising, sample */
	SPI_TRANSFER_MODE_3 = SERCOM_SPI_CTRLA_CPHA | SERCOM_SPI_CTRLA_CPOL,
};		
						  
void sercom_spi_init(void);
void spi_txrx_data(uint8_t *txrx_data);

#endif // samr21_spi_h__
