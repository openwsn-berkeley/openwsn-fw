/**
\brief GINA-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
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
   // reset local variables
   memset(&uart_vars,0,sizeof(uart_vars_t));
   
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

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
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

kick_scheduler_t uart_tx_isr() {
   uart_clearTxInterrupts(); // TODO: do not clear, but disable when done
   uart_vars.txCb();
   return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr() {
   uart_clearRxInterrupts(); // TODO: do not clear, but disable when done
   uart_vars.rxCb();
   return DO_NOT_KICK_SCHEDULER;
}