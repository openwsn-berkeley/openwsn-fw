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
/* From OpenWSN */
#include "uart.h" 
#include "samr21_uart.h"


/* === MACROS ============================================================== */
#define USART_HOST_BAUDRATE  (UART_BAUDRATE_115200)

/* === GLOBALS ============================================================= */
typedef struct {
	uart_tx_cbt txCb;
	uart_rx_cbt rxCb;
	uint8_t     startOrend;
	uint8_t     flagByte;
} uart_vars_t;

uart_vars_t uart_vars;

volatile uint8_t uart_rx_byte;

void usart_write_callback(void);
void usart_read_callback(void);

/*
 * @brief uart_init Initialize the UART Functions with required baudrate 
 *
 * @param None
 *
 */
void uart_init(void)
{
  usart_init(USART_HOST_BAUDRATE);
}

/*
 * @brief uart_setCallbacks register the callbacks
 * @param txCb uart tx call back function pointer
 * @param rxCb uart rx call back function pointer 
 *
 *
 */
void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb)
{
	/* Register Callbacks */
	uart_vars.txCb = txCb;
	uart_vars.rxCb = rxCb;
	uart_irq_enable();
}

/*
 * @brief usart_write_callback usart write callback 
 *        from the TXC interrupt
 *
 * @param None
 *
 */
void usart_write_callback(void)
{
	uart_tx_isr();
}

/*
 * @brief usart_read_callback usart read callback 
 *        from the UART RXC interrupt
 *
 * @param None
 *
 */
void usart_read_callback(void)
{
   uart_rx_isr();
}

/*
 * @brief uart_enableInterrupts This function will enable
 *        usart TXC and RXC interrupts
 *
 * @param None
 *
 */
void uart_enableInterrupts(void)
{
  usart_enable_txrx_isr();
}

/*
 * @brief uart_disableInterrupts This function will disable
 *        usart TXC and RXC interrupts
 *
 * @param None
 *
 */
void uart_disableInterrupts(void)
{
  usart_disable_txrx_isr();
}

/*
 * @brief uart_writeByte This function will write the
 *        one byte to usart Data Register
 *
 * @param None
 *
 */
void uart_writeByte(uint8_t byteToWrite)
{  
  uart_write_byte(byteToWrite);
}

/*
 * @brief uart_readByte This function will read
 *        one byte from usart Data Register
 *
 * @param None
 *
 */
uint8_t uart_readByte(void)
{
  /* Return the received data */
  return uart_rx_byte;
}

/*
 * @brief uart_clearTxInterrupts This function will clear
 *        usart TXC interrupt flag
 *
 * @param None
 *
 */
void uart_clearTxInterrupts(void)
{
  uart_clear_tx_flag();
}

/*
 * @brief uart_clearRxInterrupts This function will clear
 *        usart RXC interrupt flag
 *
 * @param None
 *
 */
void uart_clearRxInterrupts(void)
{
  uart_clear_rx_flag();	
}

/*
 * @brief uart_tx_isr This function will be called from usart
 *        interrupt when TXC interrupt flag is set
 *
 * @param return kick_scheduler_t
 *
 */
kick_scheduler_t uart_tx_isr(void) 
{
  uart_clearTxInterrupts();
  if(uart_vars.txCb != NULL)
  {
	uart_vars.txCb();    
  } 
 return DO_NOT_KICK_SCHEDULER;  
}

/*
 * @brief uart_rx_isr This function will be called from usart
 *        interrupt when RTXC interrupt flag is set
 *
 * @param return kick_scheduler_t
 *
 */
kick_scheduler_t uart_rx_isr(void)
{
	uart_clearRxInterrupts();
	if (uart_vars.rxCb != NULL)
	{
		uart_vars.rxCb();  
	}
  return DO_NOT_KICK_SCHEDULER;  
}
