/**
\brief Zigduino definition of the "uart" bsp module.

\author Sven Akkermans <sven.akkermans@cs.kuleuven.be>, September 2015.
 */

#include "uart.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
	uart_tx_cbt txCb;
	uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init() {
	PRR0 &= ~(1<<PRUSART0); //enable usart0, according to pg 343

	memset(&uart_vars,0,sizeof(uart_vars_t));	// reset local variables

	UBRR0H =  UBRRH_VALUE; 	//UBRRnH contains the baud rate
	UBRR0L = UBRRL_VALUE;

	UCSR0A |= (1<<TXC0) | (1<<RXC0); //Clear tx/rx complete bits

	if(USE_2X){
		UCSR0A |= (1<<U2X0);
	}

	UCSR0B = (1<<RXCIE0) | (1<<TXCIE0) // Enable rx&tx interrupt,
			| (1<< RXEN0) | (1<<TXEN0);	// enable rx&tx
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);	// async usart, no parity, 1-bit stop, 8-bit mode
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
	uart_vars.txCb = txCb;
	uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
	UCSR0B |= (1<<RXCIE0) | (1<<TXCIE0);
}

void    uart_disableInterrupts(){
	UCSR0B &= ~(1<<RXCIE0);
	UCSR0B &= ~(1<<TXCIE0);
}

void    uart_clearRxInterrupts(){
	UCSR0A |= (1<<RXC0);
}

void    uart_clearTxInterrupts(){
	UCSR0A |=  (1<<TXC0);
}

void    uart_writeByte(uint8_t byteToWrite){
	while((UCSR0A & _BV(UDRE0))==0);
	UDR0 = byteToWrite;
}

uint8_t uart_readByte(){
	return UDR0;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr() {
	if(uart_vars.txCb)
		uart_vars.txCb();
	return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr() {
	char dummy;
	if (uart_vars.rxCb)
		uart_vars.rxCb();
	if (RXC0) {dummy = UDR0;} 	// make sure buffer was read
	return DO_NOT_KICK_SCHEDULER;
}
