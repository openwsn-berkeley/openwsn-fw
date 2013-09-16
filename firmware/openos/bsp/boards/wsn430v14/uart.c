/**
\brief wsn430v14-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
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
 
  // Selecting the I/O pins for UART1

  P3SEL |= BIT6 + BIT7;

  // Enabling UART1 TX and RX 
   
  ME2 |= URXE1 + UTXE1;

  // Selecting character format

  UCTL1 |= CHAR;

  // Sourcing UART module with a clock at a wanted frequency : 4.8MHz SMCLK, 115200 baud

  UTCTL1 |= SSEL1;

  // Setting baud rate generator 
  //4.8 MHz/115200 = 41.66 

  UBR01 = 0x41;
  UBR11 = 0x00;
  // UMCTL1 = 0x6D;  
  UMCTL1 = 0x4A;
  
  // Enabling UART1 state machine

  UCTL1 &= ~SWRST;

  
    
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
  IE2 |=  (URXIE1 | UTXIE1);  
}

void    uart_disableInterrupts(){
  IE2 &= ~(URXIE1 | UTXIE1);
}

void    uart_clearRxInterrupts(){
  IFG2   &= ~URXIFG1;
}

void    uart_clearTxInterrupts(){
  IFG2   &= ~UTXIFG1;
}

void    uart_writeByte(uint8_t byteToWrite){
  while (!(IFG2 & UTXIFG1));
  U1TXBUF = byteToWrite;
}

uint8_t uart_readByte(){
  return U1RXBUF;
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
