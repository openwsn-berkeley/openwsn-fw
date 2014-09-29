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

/* === INCLUDES ============================================================ */
#include "spi.h"
#include "samr21_gpio.h"
#include "samr21_spi.h"
#include "leds.h"
#include "delay.h"

/* === GLOBALS ============================================================= */
typedef struct {
	// information about the current transaction
	uint8_t*        pNextTxByte;
	uint8_t         numTxedBytes;
	uint8_t         txBytesLeft;
	spi_return_t    returnType;
	uint8_t*        pNextRxByte;
	uint8_t         maxRxBytes;
	spi_first_t     isFirst;
	spi_last_t      isLast;
	// state of the module
	uint8_t         busy;
#ifdef SPI_IN_INTERRUPT_MODE
	// callback when module done
	spi_cbt         callback;
#endif
} spi_vars_t;

spi_vars_t spi_vars;

#define AT86RFX_SPI_CS PIN_PB31


/*
 * @brief spi_init Initialize the SPI driver as master
 *
 * @param None
 *
 */ 
void spi_init(void)
{
	 /* clear variables */
	 memset(&spi_vars,0,sizeof(spi_vars_t)); 
	 
	 sercom_spi_init();
}

/*
 * @brief spi_init Function to read the transceiver register
 *
 * @param addr transceiver register address
 *
 * @param return uint8_t transceiver register value
 *
 */ 
uint8_t trx_reg_read(uint8_t addr)
{

	/* Prepare the command byte */
	addr |= 0x80;

	/* Start SPI transaction by pulling SEL low */
	port_pin_set_level(AT86RFX_SPI_CS, SET_LOW);

	/* Send the Read command byte */
	spi_txrx_data(&addr);
	addr = 0;
	spi_txrx_data(&addr);
	
	/* Stop the SPI transaction by setting SEL high */
	port_pin_set_level(AT86RFX_SPI_CS, SET_HIGH);

	return addr;
}

/*
 * @brief spi_setCallback Register the SPI in call back mode
 *
 * @param cb callback function pointer
 *
 */
#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCallback(spi_cbt cb)
{
 spi_vars.callback = cb;
}
#endif

/*
 * @brief spi_txrx SPI transmit and receive the data
 *
 * @param bufTx Transmit Buffer
 * @param lenbufTx Length of transmit buffer
 * @param returnType
 * @param bufRx SPI Rx Buffer
 * @param Max Length of Rx Buffer size
 * @param isFirst 
 * @param isLast
 *
 */
void spi_txrx(uint8_t*  bufTx,
			 uint8_t      lenbufTx,
			 spi_return_t returnType,
			 uint8_t*     bufRx,
			 uint8_t      maxLenBufRx,
			 spi_first_t  isFirst,
             spi_last_t   isLast)
{
   uint8_t spi_byte;
#ifdef SPI_IN_INTERRUPT_MODE	
   /* Disable the interrupts */
   cpu_irq_disable();
#endif
   
   /* register spi frame to send */
   spi_vars.pNextTxByte      =  bufTx;
   spi_vars.numTxedBytes     =  0;
   spi_vars.txBytesLeft      =  lenbufTx;
   spi_vars.returnType       =  returnType;
   spi_vars.pNextRxByte      =  bufRx;
   spi_vars.maxRxBytes       =  maxLenBufRx;
   spi_vars.isFirst          =  isFirst;
   spi_vars.isLast           =  isLast;
   
   /* SPI is now busy */
   spi_vars.busy             =  1;
   
   /* lower CS signal to have slave listening */
   if (spi_vars.isFirst==SPI_FIRST) {
	  /* Start SPI transaction by pulling SEL low */
	 port_pin_set_level(AT86RFX_SPI_CS, SET_LOW);
	 cpu_delay_us(10);

   }
   
#ifdef SPI_IN_INTERRUPT_MODE
   /* implementation 1. use a callback function when transaction finishes */
   
   /* Send the Read command byte */	
   
   spi_write_buffer_wait(&master, spi_vars.pNextTxByte, 1);
   
   /* re-enable interrupts */
   cpu_irq_enable();
#else
   /* implementation 2. busy wait for each byte to be sent */
   /* send all bytes */
   while (spi_vars.txBytesLeft > 0) 
   {
	   /* write next byte to TX buffer */
	   /* Send the Read command byte */
		spi_byte = *spi_vars.pNextTxByte;
		spi_txrx_data(&spi_byte);

	   /* save the byte just received in the RX buffer */
	   switch (spi_vars.returnType) {
		   case SPI_FIRSTBYTE:
		   if (spi_vars.numTxedBytes==0) {				
				*spi_vars.pNextRxByte = spi_byte;			   
		   }
		   break;
		   case SPI_BUFFER:		   
		   *spi_vars.pNextRxByte = spi_byte;
		   spi_vars.pNextRxByte++;
		   break;
		   case SPI_LASTBYTE:		   
		   *spi_vars.pNextRxByte = spi_byte;
		   break;
	   }
	   /* one byte less to go */
	   spi_vars.pNextTxByte++;
	   spi_vars.numTxedBytes++;
	   spi_vars.txBytesLeft--;
   }
   
   /* put CS signal high to signal end of transmission to slave */
   if (spi_vars.isLast==SPI_LAST) 
   {
	   /* Stop the SPI transaction by setting SEL high */
	   port_pin_set_level(AT86RFX_SPI_CS, SET_HIGH);
   }
   
   /* SPI is not busy anymore */
   spi_vars.busy             =  0;
#endif			 
}

/*
 * @brief spi_isr SPI isr call back handler
 *
 * @param return kick_scheduler_t
 *
 */ 
kick_scheduler_t spi_isr(void)
{
#ifdef SPI_IN_INTERRUPT_MODE
/* save the byte just received in the RX buffer */
 switch (spi_vars.returnType) 
 {
	case SPI_FIRSTBYTE:
	if (spi_vars.numTxedBytes==0) 
	{
		spi_read_buffer_wait(&master, spi_vars.pNextRxByte, 1, 0);
	}
	break;
	
	case SPI_BUFFER:
	spi_read_buffer_wait(&master, spi_vars.pNextRxByte, 1, 0);
	spi_vars.pNextRxByte++;
	break;
	
	case SPI_LASTBYTE:
	spi_read_buffer_wait(&master, spi_vars.pNextRxByte, 1, 0);
	break;
 }

 /* one byte less to go */
 spi_vars.pNextTxByte++;
 spi_vars.numTxedBytes++;
 spi_vars.txBytesLeft--;

 if (spi_vars.txBytesLeft>0) {
	
	/* write next byte to TX buffer */
	spi_write_buffer_wait(&master, spi_vars.pNextTxByte, 1);
	} else {
		
	/* put CS signal high to signal end of transmission to slave */
	if (spi_vars.isLast==SPI_LAST) {
		
		/* Stop the SPI transaction by setting SEL high */
		spi_select_slave(&master, &slave, false);
	}
	
	/* SPI is not busy anymore */
	spi_vars.busy             =  0;
	
	/* SPI is done! */
	if (spi_vars.callback!=NULL) {
		
		/* call the callback */
		spi_vars.callback();
		
		/* kick the OS */
		return KICK_SCHEDULER;
	}
}
return DO_NOT_KICK_SCHEDULER;

#else

/* this should never happen! */
while(1);

/* we can not print from within the BSP. Instead we
   blink the error LED */
leds_error_blink();

/* reset the board */
board_reset();

 /* execution will not reach here, statement to make compiler happy */
return DO_NOT_KICK_SCHEDULER;
#endif
}

