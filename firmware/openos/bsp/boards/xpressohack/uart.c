



/**
\brief LPC17XX-specific definition of the "UART" bsp module. Note that this module only enables UART0.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "uart_config.h"
#include "LPC17xx.h"
#include "clkpwr.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================
extern void UART0_IRQHandler (void); //weak function defined in cr_startup_lpc17.c
static void private_determinePCLK(uint32_t pclkdiv, uint32_t *pclk);


//=========================== public ==========================================

void uart_init() {
    uint32_t baudrate;
	uint32_t Fdiv;
	uint32_t pclkdiv, pclk;

	// reset local variables
	memset(&uart_vars,0,sizeof(uart_vars_t));

#ifdef UART_BAUDRATE_115200
	baudrate=115200;
#else
	baudrate=9600;
#endif

	LPC_PINCON->PINSEL0 &= ~0x000000F0;
	LPC_PINCON->PINSEL0 |= 0x00000050;  /* RxD0 is P0.3 and TxD0 is P0.2 */
	/* By default, the PCLKSELx value is zero, thus, the PCLK for
		all the peripherals is 1/4 of the SystemCoreClock. */
	/* Bit 6~7 is for UART0 */
	pclkdiv = (LPC_SC->PCLKSEL0 >> 6) & 0x03;

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_UART0,CLKPWR_PCLKSEL_CCLK_DIV_4);//default clock


	LPC_UART0->LCR = 0x83;		/* DLAB=1 , 8 bits, no Parity, 1 Stop bit
	 *  0b10000011
	 *    ||||||||__ Word Length select low
	 *    |||||||___ Word Length select high (set to 11 --> 8bit char length)
	 *    ||||||____ Stop bit select (0=1stop bit, 1=2 stop bits)
	 *    |||||_____ Parity enable (0=disable parity generation and check, 1=enable it)
	 *    ||||______ Parity select low
	 *    |||_______ Parity select high
	 *    ||________ Break Control
	 *    |_________ Divisor Latch DLAB = 0
	 */
	Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
	LPC_UART0->DLM = Fdiv / 256; /*higher 8 bits of the divisor -- divides pclk in order to get the desired baudrate. see p.301.*/
	LPC_UART0->DLL = Fdiv % 256; /*lower 8 bits of the divisor*/
	LPC_UART0->LCR = 0x03;		/* UART line control register DLAB = 0 set after configuring baudrate. See manual p.298.
	 *  0b00000011
	 *    ||||||||__ Word Length select low
	 *    |||||||___ Word Length select high (set to 11 --> 8bit char length)
	 *    ||||||____ Stop bit select (0=1stop bit, 1=2 stop bits)
	 *    |||||_____ Parity enable (0=disable parity generation and check, 1=enable it)
	 *    ||||______ Parity select low
	 *    |||_______ Parity select high
	 *    ||________ Break Control
	 *    |_________ Divisor Latch DLAB = 0
	 */

	LPC_UART0->FCR = 0x07;		/*0x07 Enable and reset TX and RX FIFO. page.305
	 *  0b00000111
	 *    ||||||||__ FIFO Enable (enables Rx Data Available interrupt)
	 *    |||||||___ RX FIFO Reset
	 *    ||||||____ TX FIFO Reset
	 *    |||||_____ DMA Mode Select - see section 14.4.6.1
	 *    ||||______ Reserved
	 *    |||_______ Reserved
	 *    ||________ RX Trigger low (for DMA)
	 *    |_________ RX Trigger high
	 */
	LPC_UART0->FCR |= (0 << 6);// Set FIFO to trigger when at least 1 characters available (only needed if FIFO is enabled) - p.305, 00-means 1 char,01 - 4chars..
	NVIC_EnableIRQ(UART0_IRQn);

	return;
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
	LPC_UART0->IER |= IER_THRE|IER_RBR| IER_RLS;	/* Enable UART0 interrupt
		 *  0b00000111
		 *    ||||||||__ RBR Interrupt Enable (enables Rx Data Available interrupt)
		 *    |||||||___ THRE Interrupt
		 *    ||||||____ Rx Line Status interrupt
		 *    |||||_____ Reserved
		 *    ||||______ Reserved
		 *    |||_______ Reserved
		 *    ||________ Reserved
		 *    |_________ ABEOIntEn (Enables the end of auto-baud interrupt)*/
}

void    uart_disableInterrupts(){
  LPC_UART0->IER &= ~IER_RBR;	/* Disables rx UART0 interrupt*/
  LPC_UART0->IER &= ~IER_THRE;	/* Disables tx UART0 interrupt*/
  LPC_UART0->IER &= ~IER_RLS;	/* Disables line Status UART0 interrupt

		 *  0b00000111
		 *    ||||||||__ RBR Interrupt Enable (enables Rx Data Available interrupt)
		 *    |||||||___ THRE Interrupt
		 *    ||||||____ Rx Line Status interrupt
		 *    |||||_____ Reserved
		 *    ||||______ Reserved
		 *    |||_______ Reserved
		 *    ||________ Reserved
		 *    |_________ ABEOIntEn (Enables the end of auto-baud interrupt)
		 */
}

void    uart_clearRxInterrupts(){
//do nothing, are done in the isr to read the line status.
}

void    uart_clearTxInterrupts(){
//do nothing, are done in the isr to read the line status.
}

void    uart_writeByte(uint8_t byteToWrite){
	LPC_UART0->THR = byteToWrite;//write
}

uint8_t uart_readByte(){
  return LPC_UART0->RBR;
}

//=========================== interrupt handlers ==============================

uint8_t uart_isr_tx() {
   uart_clearTxInterrupts(); // TODO: do not clear, but disable when done
   uart_vars.txCb();
   return 0;
}

uint8_t uart_isr_rx() {
   uart_clearRxInterrupts(); // TODO: do not clear, but disable when done
   uart_vars.rxCb();
   return 0;
}



void UART0_IRQHandler (void)
{
	//poipoiportBASE_TYPE xHigherPriorityTaskWoken=pdFALSE;
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;

	IIRValue = LPC_UART0->IIR;
	/* IIR register. clears the interrupt when read. p.303
		 *  0b10000011
		 *    ||||||||__ IntStatus - 0 if at least one pending interrupt. 1 otherwise.
		 *    |||||||___ Interrupt id 011-RLS,010-RDA,110-CTI,001-THRE interrupt.
		 *    ||||||____ Interrupt id 011-RLS,010-RDA,110-CTI,001-THRE interrupt.
		 *    |||||_____ Interrupt id 011-RLS,010-RDA,110-CTI,001-THRE interrupt.
		 *    ||||______ Reserved
		 *    |||_______ Reserved
		 *    ||________ Fifo Enable. copies of UnFCR[0]
		 *    |_________ Fifo Enable. copies of UnFCR[0]
		 */


	IIRValue >>= 1;			/* skip pending bit in IIR */
	IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
	if ( IIRValue == IIR_RLS )		/* Receive Line Status */
	{
		LSRValue = LPC_UART0->LSR;
		/* Receive Line Status */
		if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{
			/* There are errors or break interrupt */
			/* Read LSR will clear the interrupt */
			Dummy = LPC_UART0->RBR;		/* Dummy read on RX to clear
							interrupt, then bail out */
			return;
		}
		if ( LSRValue & LSR_RDR )	/* Receive Data Ready */
		{
			/* If no error on RLS, normal ready, save into the data buffer. */
			/* Note: read RBR will clear the interrupt */
			uart_isr_rx();//call isr rx wrapper.

		}
	}
	else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
	{
		/* Receive Data Available */
		uart_isr_rx();//call isr rx wrapper.
	}
	else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
	{
		/* Character Time-out indicator */
     	//do nothing.
	}
	else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
	{
		/* THRE interrupt */
		LSRValue = LPC_UART0->LSR;		/* Check status in the LSR to see if
									valid data in U0THR or not */
		if ( LSRValue & LSR_THRE )
		{
			//call the isr tx wrapper.
			uart_isr_tx();
		}
		else
		{
			//not ready yet.. nothing to do??
		}
	}
}

