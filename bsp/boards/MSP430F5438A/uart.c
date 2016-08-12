/**
\brief msp430f5438a-specific definition of the "uart" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, August 2016.
*/

#include "msp430f5438a.h"
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
   
   //P3SEL                     =  0x30;            // P3.4,5 = = USCI_A0 TXD/RXD
   P5SEL                       =  0xc0;            //P5.6,7 = USCI_A1
  
   UCA1CTL1                  |=  UCSWRST;           // **Put state machine in reset**
   UCA1CTL1                  |=  UCSSEL_2;          // SMCLK
   
   UCA1BR0 = 0xd9;                                 // ~26.5MHz/115200 = 230 = 0xe6
   UCA1BR1 = 0;
   UCA1MCTL |= UCBRS_1 + UCBRF_0;
   UCA1CTL1 &= ~UCSWRST;                          // **Initialize USCI state machine**

}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
  UCA1IE |= UCRXIE | UCTXIE ;  
}

void    uart_disableInterrupts(){
  UCA1IE &= ~(UCRXIE | UCTXIE);
}

void    uart_clearRxInterrupts(){
  UCA1IFG   &= ~UCRXIFG;
}

void    uart_clearTxInterrupts(){
  UCA1IFG   &= ~UCTXIFG;
}

void    uart_writeByte(uint8_t byteToWrite){
  UCA1TXBUF = byteToWrite;
}

uint8_t uart_readByte(){
  return UCA1RXBUF;
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
