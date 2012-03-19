/**
\brief GINA-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "openserial.h"

//=========================== defines =========================================
//=========================== variables =======================================
//=========================== prototypes ======================================
//=========================== public ==========================================

void uart_init() {
  //initialize UART openserial_vars.mode
   P3SEL    |=  0xC0;                             // P3.6,7 = USCI_A1 TXD/RXD
   UCA1CTL1 |=  UCSSEL_2;                         // CLK = SMCL
   UCA1BR0   =  0x8a;                             // 115200 baud if SMCLK@16MHz
   UCA1BR1   =  0x00;
   UCA1MCTL  =  UCBRS_7;                          // Modulation UCBRSx = 7
   UCA1CTL1 &= ~UCSWRST;                          // Initialize USCI state machine
   //UC1IFG   &= ~(UCA1TXIFG | UCA1RXIFG);          // clear possible pending interrupts
   //UC1IE    |=  (UCA1RXIE  | UCA1TXIE);           // Enable USCI_A1 TX & RX interrupt  
}


void    uart_enableInterrupts(){
  UC1IE    |=  (UCA1RXIE  | UCA1TXIE);  
}

void    uart_disableInterrupts(){
  UC1IE &= ~(UCA1RXIE | UCA1TXIE);
}

void    uart_clearRxInterrupts(){
  UC1IFG   &= ~(UCA1RXIFG);
}

void    uart_clearTxInterrupts(){
  UC1IFG   &= ~(UCA1TXIFG);
}

void    uart_writeByte(uint8_t byteToWrite){
  UCA1TXBUF = byteToWrite;
}

uint8_t uart_readByte(){
  return UCA1RXBUF;
}

//=========================== interrupt handlers ==============================

uint8_t uart_isr_tx() {
   uart_clearTxInterrupts(); // TODO: do not clear, but disable when done
   isr_openserial_tx();
   return 0;
}

uint8_t uart_isr_rx() {
   uart_clearRxInterrupts(); // TODO: do not clear, but disable when done
   isr_openserial_rx();
   return 0;
}