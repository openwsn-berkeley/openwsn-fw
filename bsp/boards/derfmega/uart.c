/**
\brief derfmega definition of the "uart" bsp module.

\author Kevin Weekly <kweekly@eecs.berkeley.edu>, June 2012.
*/

#include <avr/io.h>
#include "stdint.h"
#include "stdio.h"
#include "string.h"
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
	//turn on power
	PRR0 &= ~(1<<PRUSART0);
	
   // reset local variables
   memset(&uart_vars,0,sizeof(uart_vars_t));
   
   //initialize UART openserial_vars.mode
   DDRE &= ~0x01; // enable rx input
   DDRE |= 0x02; // enable tx output

   UBRR0H = 0;
   UBRR0L = 16; // results in baud rate of 117647
   UCSR0A = 0b01000010; // double speed mode
   UCSR0B = 0b11011000; // enable RX/TX, interrupts, 8-bit mode
   UCSR0C = 0b00000110; // enable 8-bit mode, no parity, etc
   
   uart_writeByte('A');
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){ 
	UCSR0B |= 0xC0;
}

void    uart_disableInterrupts(){
	UCSR0B &= ~0xC0;
}

void    uart_clearRxInterrupts(){
}

void    uart_clearTxInterrupts(){
  UCSR0A |= 0x40;
}

void    uart_writeByte(uint8_t byteToWrite){
  while((UCSR0A & _BV(UDRE0))==0);
  UDR0 = byteToWrite;
}

uint8_t uart_readByte(){
  return UDR0;
}

//=========================== interrupt handlers ==============================

uint8_t uart_isr_tx() {
   if(uart_vars.txCb)
		uart_vars.txCb();
   return 0;
}

uint8_t uart_isr_rx() {
	char dummy;
	if (uart_vars.rxCb)
		uart_vars.rxCb();
   // make sure buffer was read
   if (RXC0) {dummy = UDR0;}
   return 0;
}