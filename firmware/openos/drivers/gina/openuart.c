#include "openuart.h"
#include "msp430x26x.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void openuart_init() {
   //initialize UART openserial_vars.mode
   P3SEL    |=  0xC0;                             // P3.6,7 = USCI_A1 TXD/RXD
   UCA1CTL1 |=  UCSSEL_2;                         // CLK = SMCL
   UCA1BR0   =  0x8a;                             // 115200 baud if SMCLK@16MHz
   UCA1BR1   =  0x00;
   UCA1MCTL  =  UCBRS_7;                          // Modulation UCBRSx = 7
   UCA1CTL1 &= ~UCSWRST;                          // Initialize USCI state machine
   //UC1IFG &= ~(UCA1TXIFG | UCA1RXIFG);          // clear possible pending interrupts
   //UC1IE  |=  (UCA1RXIE  | UCA1TXIE);           // Enable USCI_A1 TX & RX interrupt  
}

void openuart_clearInt() {
   UC1IFG   &= ~(UCA1TXIFG | UCA1RXIFG);          // clear possible pending interrupts
   UC1IE    |=  (UCA1RXIE  | UCA1TXIE);           // Enable USCI_A1 TX & RX interrupt
}

void openuart_clearIntOnly() {
   UC1IE &= ~(UCA1RXIE | UCA1TXIE);              // disable USCI_A1 TX & RX interrupt
}

void openuart_clearTxFlag() {
   UC1IFG &= ~UCA1TXIFG; // TODO: do not clear, but disable when done
}

void openuart_clearRxFlag() {
   UC1IFG &= ~UCA1RXIFG; // TODO: do not clear, but disable when done
}

void openuart_txByte(uint8_t byteToTx) {
   UCA1TXBUF = byteToTx;
}

uint8_t openuart_getRxByte() {
   return UCA1RXBUF;
}

//=========================== private =========================================
