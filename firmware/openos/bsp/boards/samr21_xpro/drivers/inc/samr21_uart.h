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

#ifndef samr21_uart_h__
#define samr21_uart_h__

#include "sam.h"
#include "cm0plus_interrupt.h"

/* Start of UART Configuration */
#define UART_MODULE              SERCOM0
#define UART_PM_MODULE			 PM_APBCMASK_SERCOM0
#define UART_GCLK				 GCLK_GENERATOR_0
#define UART_MODULE_GCLK	     SERCOM0_GCLK_ID_CORE
#define UART_SERCOM_MUX_SETTING  (SERCOM_USART_CTRLA_RXPO(1))
#define UART_TX_MUX_PIN			 MUX_PA04D_SERCOM0_PAD0
#define UART_RX_MUX_PIN			 MUX_PA05D_SERCOM0_PAD1
#define UART_TX_PIN				 PIN_PA04
#define UART_RX_PIN				 PIN_PA05
#define F_UART					(1024ul * 32768ul)
/* End of UART Configuration */

/**
 * \brief USART Stop Bits enum
 *
 * Number of stop bits for a frame.
 */
enum usart_stopbits {
	/** Each transferred frame contains 1 stop bit */
	USART_STOPBITS_1 = 0,
	/** Each transferred frame contains 2 stop bits */
	USART_STOPBITS_2 = SERCOM_USART_CTRLB_SBMODE,
};

/* uart baudrate value */
typedef enum
{
	UART_BAUDRATE_9600 = ((uint64_t)65536ul * (F_UART - 16ul * 9600ul) / F_UART), // 9600 baud rate
	UART_BAUDRATE_19200 = ((uint64_t)65536ul * (F_UART - 16ul * 19200ul) / F_UART), // 19200 baud rate
	UART_BAUDRATE_38400 = ((uint64_t)65536ul * (F_UART - 16ul * 38400ul) / F_UART), // 38400 baud rate
	UART_BAUDRATE_57600 = ((uint64_t)65536ul * (F_UART - 16ul * 57600ul) / F_UART), // 57600 baud rate
	UART_BAUDRATE_115200 =((uint64_t)65536ul * (F_UART - 16ul * 115200ul)/ F_UART) // 115200 baud rate
} uart_baudrate_t;

/**
 * \brief USART Data Order enum
 *
 * The data order decides which of MSB or LSB is shifted out first when data is
 * transferred
 */
enum usart_dataorder {
	/** The MSB will be shifted out first during transmission,
	 *  and shifted in first during reception */
	USART_DATAORDER_MSB = 0,
	/** The LSB will be shifted out first during transmission,
	 *  and shifted in first during reception */
	USART_DATAORDER_LSB = SERCOM_USART_CTRLA_DORD,
};

/**
 * \brief USART Transfer mode enum
 *
 * Select USART transfer mode
 */
enum usart_transfer_mode {
	/** Transfer of data is done synchronously */
	USART_TRANSFER_SYNCHRONOUSLY = (SERCOM_USART_CTRLA_CMODE),
	/** Transfer of data is done asynchronously */
	USART_TRANSFER_ASYNCHRONOUSLY = 0
};

/**
 * \brief USART Sample Adjustment
 *
 * The value of sample number used for majority voting
 */
enum usart_sample_adjustment {
	/** The first, middle and last sample number used for majority voting is 7-8-9 */
	USART_SAMPLE_ADJUSTMENT_7_8_9 = SERCOM_USART_CTRLA_SAMPA(0),
	/** The first, middle and last sample number used for majority voting is 9-10-11 */
	USART_SAMPLE_ADJUSTMENT_9_10_11 = SERCOM_USART_CTRLA_SAMPA(1),
	/** The first, middle and last sample number used for majority voting is 11-12-13 */
	USART_SAMPLE_ADJUSTMENT_11_12_13 = SERCOM_USART_CTRLA_SAMPA(2),
	/** The first, middle and last sample number used for majority voting is 13-14-15 */
	USART_SAMPLE_ADJUSTMENT_13_14_15 = SERCOM_USART_CTRLA_SAMPA(3),
};


/**
 * \brief Enable usart interrupt vector
 *
 * \param[in] None
 */
static inline void uart_irq_enable(void)
{
	nvic_irq_enable(SERCOM0_IRQn);
}


/** \brief uart_clear_tx_flag This function will clears the Tx Interrupt Flag if occurred

     

    \param [in] None
    \return None
 */
static inline void uart_clear_tx_flag(void)
{
 Sercom *const hw = UART_MODULE;
 hw->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;
}

/** \brief uart_clear_rx_flag This function will clears the Rx Interrupt Flag if occurred

     

    \param [in] None
    \return None
 */
static inline void uart_clear_rx_flag(void)
{
 Sercom *const hw = UART_MODULE;
 hw->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_RXC;
}



/** \brief usart_enable_txrx_isr This function enables the usart interrupts

     

    \param [in] None
    \return None
 */
static inline void usart_enable_txrx_isr(void)
{
 Sercom *const hw = UART_MODULE;
 while((hw->USART.SYNCBUSY.reg)){
	 /* Wait until the synchronization completes */
 }
 hw->USART.INTENSET.reg = SERCOM_USART_INTFLAG_TXC | SERCOM_USART_INTFLAG_RXC;
}

/** \brief usart_disable_txrx_isr This function disables the usart interrupts

     

    \param [in] None
    \return None
 */
static inline void usart_disable_txrx_isr(void)
{
 Sercom *const hw = UART_MODULE;
 while(hw->USART.SYNCBUSY.reg){
	 /* Wait until the synchronization completes */
 }
 hw->USART.INTENCLR.reg = SERCOM_USART_INTFLAG_TXC | SERCOM_USART_INTFLAG_RXC;
}

void usart_init(uint32_t baudrate);
uint8_t uart_write_byte(uint8_t byte);
uint8_t uart_read_byte(uint8_t *byte);

#endif // samr21_uart_h__
