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
#include "samr21_uart.h"
#include "samr21_sysctrl.h"
#include "samr21_gclk.h"
#include "samr21_gpio.h"
#include "base_type.h"

/* To Store the Received Byte and avoid the overrun error */
volatile uint8_t uart_rx_byte;

void usart_write_callback(void) __attribute__ ((weak, alias("dummy_irq")));
void usart_read_callback(void) __attribute__ ((weak, alias("dummy_irq")));

void usart_init(uint32_t baudrate)
{
	Sercom *const hw = UART_MODULE;
	pinmux_t config;

	/* Get a pointer to the hardware module instance */
	SercomUsart *const usart_hw = &(hw->USART);
	
	
	while(usart_hw->CTRLA.reg & SERCOM_USART_CTRLA_SWRST){
		/* The module is busy resetting itself */
	}
	
	while(usart_hw->CTRLA.reg & SERCOM_USART_CTRLA_ENABLE){
		/* Check the module is enabled */
	}
	
	/* Turn on module in PM */
	PM->APBCMASK.reg |= UART_PM_MODULE;
	
	/* Set up the GCLK for the module */
	gclk_chan_config(UART_MODULE_GCLK, UART_GCLK);
	gclk_enable(UART_MODULE_GCLK);
	gclk_chan_config(SERCOM_GCLK_ID, UART_GCLK);
	gclk_enable(SERCOM_GCLK_ID);
	
	/* Configure the SERCOM pins according to the configuration */
	config.dir = PORT_PIN_DIR_INPUT;
	config.pull =  PORT_PIN_PULLNONE;
	config.mux_loc = UART_TX_MUX_PIN & 0xFFFF;
	pinmux_config(UART_TX_PIN, &config);
	config.mux_loc = UART_RX_MUX_PIN & 0xFFFF;
	pinmux_config(UART_RX_PIN, &config);
	
	/* Wait until synchronization is complete */
	while(usart_hw->SYNCBUSY.reg);
	
	/*Set baud val */
	usart_hw->BAUD.reg = baudrate;
	
	/* Wait until synchronization is complete */
	while(usart_hw->SYNCBUSY.reg);
	
    /* Set stop bits, character size and enable transceivers */
	/* Write configuration to CTRLB */
	usart_hw->CTRLB.reg = USART_STOPBITS_1 | SERCOM_USART_CTRLB_CHSIZE(0);
	

	/* Wait until synchronization is complete */
	while(usart_hw->SYNCBUSY.reg);

	/* Write configuration to CTRLA */
	usart_hw->CTRLA.reg = USART_TRANSFER_ASYNCHRONOUSLY | SERCOM_USART_CTRLA_MODE_USART_INT_CLK |\
						  USART_DATAORDER_LSB | UART_SERCOM_MUX_SETTING;
						  
	/* Wait until synchronization is complete */
	while(usart_hw->SYNCBUSY.reg);

	/* Write configuration to CTRLA */
	usart_hw->CTRLB.reg |= SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN;
	
   /* Wait until synchronization is complete */
	while(usart_hw->SYNCBUSY.reg);
	/* Write configuration to CTRLA */
	usart_hw->CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;	
}

/** \brief uart_write_byte This function writes one byte into UART for transmission

     

    \param [in] byte 
    \return status true or false
 */
uint8_t uart_write_byte(uint8_t byte)
{
 uint16_t tx_byte = byte;	
 Sercom *const hw = UART_MODULE;
 while(!(hw->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE));
 while((hw->USART.SYNCBUSY.reg)){
	 /* Wait until the synchronization completes */
 }
 hw->USART.DATA.reg = tx_byte;
 return true;
}


/** \brief uart_read_byte This function reads one byte from UART

     

    \param [in] *byte 
    \return status true or false
 */
uint8_t uart_read_byte(uint8_t *byte)
{
  /* Return the received data */
  *byte = uart_rx_byte;
  return true;
}

/** \brief SERCOM0_Handler This function handles the USART Interrupt

     

    \param [in] None
    \return None
 */
void SERCOM0_Handler(void)
{
 Sercom *const hw = UART_MODULE;
 while(hw->USART.SYNCBUSY.reg){
	 /* Wait until the synchronization completes */
 }	
 uint16_t irq_flag = hw->USART.INTFLAG.reg;
 /* mask the enabled interrupt bits */
 
 if (irq_flag & SERCOM_USART_INTFLAG_DRE)
 {
	 hw->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_DRE;
	 /* Add the callback here */
 }
 
 if(irq_flag & SERCOM_USART_INTFLAG_TXC)
 {
	 hw->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;
	 /* Add the callback here */
	 usart_write_callback();
 }
 
 if (irq_flag & SERCOM_USART_INTFLAG_RXC)
 {
	 hw->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_RXC;
	 /* Check the error status and clear if it's occurred */
	 uint8_t error_code = (uint8_t)(hw->USART.STATUS.reg & SERCOM_USART_STATUS_MASK);
	 if(error_code)
	 {
		if (error_code & SERCOM_USART_STATUS_FERR) 
		{
			/* clear flag by writing 1 to it */
			hw->USART.STATUS.reg |= SERCOM_USART_STATUS_FERR;
		} 
		else if (error_code & SERCOM_USART_STATUS_BUFOVF) 
		{
			/* clear flag by writing 1 to it */
			hw->USART.STATUS.reg |= SERCOM_USART_STATUS_BUFOVF;
		} 
		else if (error_code & SERCOM_USART_STATUS_PERR) 
		{
			/* clear flag by writing 1 to it */
			hw->USART.STATUS.reg |= SERCOM_USART_STATUS_PERR;
		}
	 }
	 else
	 {
		 uint16_t rxd_data = hw->USART.DATA.reg;
		 uart_rx_byte = (uint8_t)rxd_data;
		 /* Add the callback here */
		 usart_read_callback();
	 }
 }
 
}




