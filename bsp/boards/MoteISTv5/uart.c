/**
\brief MoteISTv5-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
\author Diogo Guerra <diogoguerra@ist.utl.pt>, <dy090.guerra@gmail.com>, July 2015.
*/

#include "hal_MoteISTv5.h"
#include "uart.h"
#include "board.h"

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
  P3SEL |=  0x30;             // P3.4,5 = USCI_A0 TX/RX
   
  UCA0CTL1 |= UCSWRST;        // **Put state machine in reset**
   
  // 115200 baud, clocked from 25MHz SMCLK
  UCA0CTL1 |= UCSSEL_2;       // SMCLK 
  UCA0BR0 = 0xD9;             // 25MHz/115200baud - 217 => 0xD9 
  UCA0BR1 = 0x00;             // (see http://mspgcc.sourceforge.net/baudrate.html)
  UCA0MCTL = 0x00;            // Modulation UCBRSx=1, UCBRFx=0
   
  UCA0IE |= UCRXIE | UCTXIE;  // Enable USCI_A0 RX interrupt
  UCA0CTL1 &= ~UCSWRST;       // **Initialize USCI state machine**
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
  UCA0IE |=  (UCRXIE | UCTXIE);  
}

void    uart_disableInterrupts(){
  UCA0IE &= ~(UCRXIE | UCTXIE);
}

void    uart_clearRxInterrupts(){
  UCA0IFG   &= ~UCRXIFG;
}

void    uart_clearTxInterrupts(){
  UCA0IFG   &= ~UCTXIFG;
}

void    uart_writeByte(uint8_t byteToWrite){
  UCA0TXBUF = byteToWrite;
}

uint8_t uart_readByte(){
  return UCA0RXBUF;
}

//=========================== private =========================================

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