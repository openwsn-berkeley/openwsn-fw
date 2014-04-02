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
   
   P3SEL      =  0x30;                           // P3.4,5 = UART0TX/RX
  
   U0CTL      =  SWRST;                          // hold UART0 module in reset
   U0CTL     |=  CHAR;                           // 8-bit character
   
   //115200 baud, clocked from 4.8MHz SMCLK
   U0TCTL    |=  SSEL1;                          // clocking from SMCLK
   U0BR0      =  41;                             // 4.8MHz/115200 - 41.66
   U0BR1      =  0x00;                           //
   U0MCTL     =  0x4A;                           // modulation
   
   ME1       |=  UTXE0 + URXE0;                  // enable UART0 TX/RX
   U0CTL     &= ~SWRST;                          // clear UART1 reset bit
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
   IE1       |=  (URXIE0 | UTXIE0);
}

void    uart_disableInterrupts(){
   IE1       &= ~(URXIE0 | UTXIE0);
}

void    uart_clearRxInterrupts(){
   IFG1      &= ~URXIFG0;
}

void    uart_clearTxInterrupts(){
   IFG1      &= ~UTXIFG0;
}

void    uart_writeByte(uint8_t byteToWrite){
  U0TXBUF     = byteToWrite;
}

uint8_t uart_readByte(){
  return U0RXBUF;
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
