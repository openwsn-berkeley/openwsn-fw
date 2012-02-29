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


//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
	uint32_t        UART0Status;
	// TX
	uint8_t         UART0TxEmpty;
	uint8_t*        txBuf;
	uint8_t         txBufLen;
	uart_txDone_cbt txDone_cb;
	// RX
	uint8_t*        rxBuf;
	uint8_t         rxBufLen;
	uint8_t         rxBufFillThres;
	uint8_t*        rxBufWrPtr;
	uint8_t*        rxBufRdPtr;
	uint8_t         rxBufFill;
	uart_rx_cbt     rx_cb;
} uart_vars_t;

uart_vars_t uart_vars;



//=========================== prototypes ======================================
extern void UART0_IRQHandler (void); //weak function defined in cr_startup_lpc17.c
static void private_reset_rxBuf();
static void private_determinePCLK(uint32_t pclkdiv, uint32_t *pclk);
static void private_rcv_data_now();


void UART0_IRQHandler (void)
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;

	IIRValue = LPC_UART0->IIR;

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
			uart_vars.UART0Status = LSRValue;
			Dummy = LPC_UART0->RBR;		/* Dummy read on RX to clear
							interrupt, then bail out */
			return;
		}
		if ( LSRValue & LSR_RDR )	/* Receive Data Ready */
		{
			/* If no error on RLS, normal ready, save into the data buffer. */
			/* Note: read RBR will clear the interrupt */
			private_rcv_data_now();

		}
	}
	else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
	{
		/* Receive Data Available */
		private_rcv_data_now();
	}
	else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
	{
		/* Character Time-out indicator */
		uart_vars.UART0Status |= 0x100;		/* Bit 9 as the CTI error */
	}
	else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
	{
		/* THRE interrupt */
		LSRValue = LPC_UART0->LSR;		/* Check status in the LSR to see if
									valid data in U0THR or not */
		if ( LSRValue & LSR_THRE )
		{
			uart_vars.UART0TxEmpty = 1;

			// one byte less to go
			uart_vars.txBufLen--;
			uart_vars.txBuf++;

			if (uart_vars.txBufLen>0) {
				// send next byte
				LPC_UART0->THR = *uart_vars.txBuf;//write
			} else {
				if (uart_vars.txDone_cb!=NULL) {
					// disable UART1 TX interrupt
					LPC_UART0->IER &=  ~IER_THRE;

					// call the callback
					uart_vars.txDone_cb();
					// make sure CPU restarts after leaving interrupt
					//TODO.. check that.
				}
			}
		}
		else
		{
			uart_vars.UART0TxEmpty = 0;
			//not ready yet.. nothing to do??
		}
	}

}


void uart_init(){

	uint32_t baudrate;
	uint32_t Fdiv;
	uint32_t pclkdiv, pclk;

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

	private_determinePCLK(pclkdiv,&pclk);

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


	LPC_UART0->FCR = 0x07;		/* Enable and reset TX and RX FIFO. page.305
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
	NVIC_EnableIRQ(UART0_IRQn);


	return;

}
// TX
void uart_txSetup(uart_txDone_cbt cb){
	uart_vars.txDone_cb       = cb;               // register callback
}

//tx now..
void uart_tx(uint8_t* txBuf, uint8_t txBufLen){

	// register data to send
	uart_vars.txBuf           = txBuf;
	uart_vars.txBufLen        = txBufLen;

	//TODO Check that as This enables RX and TX interrupts.
	LPC_UART0->IER |=  IER_THRE | IER_RLS;	/* Enable UART0 interrupt
	 *  0b00000111
	 *    ||||||||__ RBR Interrupt Enable (enables Rx Data Available interrupt)
	 *    |||||||___ THRE Interrupt
	 *    ||||||____ Rx Line Status interrupt
	 *    |||||_____ Reserved
	 *    ||||______ Reserved
	 *    |||_______ Reserved
	 *    ||________ Reserved
	 *    |_________ ABEOIntEn (Enables the end of auto-baud interrupt)*/

	LPC_UART0->THR = *txBuf;//write first bit in the uart. on isr, write the next one

	return;
}


// RX
void uart_rxSetup(uint8_t*    rxBuf,uint8_t     rxBufLen, 	uint8_t     rxBufFillThres, uart_rx_cbt cb){

	uart_vars.rxBuf           = rxBuf;
	uart_vars.rxBufLen        = rxBufLen;
	uart_vars.rxBufFillThres  = rxBufFillThres;
	private_reset_rxBuf();
	uart_vars.rx_cb           = cb;
}

void uart_rxStart(){
	//TODO Check that as This enables RX and TX interrupts.
	LPC_UART0->IER |= IER_RBR  | IER_RLS;	/* Enable UART0 interrupt
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

void uart_readBytes(uint8_t* buf, uint8_t numBytes) {
	uint8_t i;

	for (i=0;i<numBytes;i++) {
		// copy byte into receive buffer
		*buf                   = *uart_vars.rxBufRdPtr;
		// advance counters
		buf++;
		uart_vars.rxBufRdPtr++;
		if (uart_vars.rxBufRdPtr>=uart_vars.rxBuf+uart_vars.rxBufLen) {
			uart_vars.rxBufRdPtr= uart_vars.rxBuf;
		}
	}

	// reduce fill
	uart_vars.rxBufFill      -= numBytes;
}

void uart_rxStop(){
	//disable UART RX interrupt
	LPC_UART0->IER &= ~IER_RBR;	/* Disables rx UART0 interrupt
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


//=========================== private =========================================

static void private_rcv_data_now(){

	// copy received by into buffer
	*uart_vars.rxBufWrPtr     =  LPC_UART0->RBR;
	// shift pointer
	uart_vars.rxBufWrPtr++;
	if (uart_vars.rxBufWrPtr>=uart_vars.rxBuf+uart_vars.rxBufLen) {
		uart_vars.rxBufWrPtr   =  uart_vars.rxBuf;
	}
	// increment fill
	uart_vars.rxBufFill++;

	if (uart_vars.rxBufFill>=uart_vars.rxBufLen) {
		// buffer has overflown
		// reset buffer
		private_reset_rxBuf();

		if (uart_vars.rx_cb!=NULL) {
			// call the callback
			uart_vars.rx_cb(UART_EVENT_OVERFLOW);
			// make sure CPU restarts after leaving interrupt
			//TODO check that...
		}

	} else if (uart_vars.rxBufFill>=uart_vars.rxBufFillThres) {
		// buffer above threshold

		if (uart_vars.rx_cb!=NULL) {
			// call the callback
			uart_vars.rx_cb(UART_EVENT_THRES);
			// make sure CPU restarts after leaving interrupt
			//TODO check that...
		}
	}

}



static void private_reset_rxBuf() {
	uart_vars.rxBufWrPtr      = uart_vars.rxBuf;
	uart_vars.rxBufRdPtr      = uart_vars.rxBuf;
	uart_vars.rxBufFill       = 0;

}

static void private_determinePCLK(uint32_t pclkdiv, uint32_t *pclk)
{
	switch (pclkdiv){
	case 0x00:
	default:
		*pclk = SystemCoreClock / 4;
		break;
	case 0x01:
		*pclk = SystemCoreClock;
		break;
	case 0x02:
		*pclk = SystemCoreClock / 2;
		break;
	case 0x03:
		*pclk = SystemCoreClock / 8;
		break;
	}
}
