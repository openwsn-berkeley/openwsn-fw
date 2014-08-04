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

void uart_init(void) {
   
   P3SEL      =  0x30;                           // P3.4,5 = UTXD0/URXD0
   ME1       |=  UTXE0 + URXE0;                  // enable UART0 TX/RX
   U0CTL     |=  CHAR;                           // 8-bit character
   U0TCTL    |=  SSEL1;                          // clocking from SMCLK
   U0BR0      =  0x08;                           // 115200 baud, using 1MHz SMCLK
   U0BR1      =  0x00;
   UMCTL0     =  0x5B;
   U0CTL     &= ~SWRST;                          // clear UART0 reset bit
   
   // clear possible pending interrupts
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(void){
   IE1       |=  (URXIE0 | UTXIE0);
}

void    uart_disableInterrupts(void){
   IE1       &= ~(URXIE0 | UTXIE0);
}

void    uart_clearRxInterrupts(void){
   IFG1      &= ~URXIFG0;
}

void    uart_clearTxInterrupts(void){
   IFG1      &= ~UTXIFG0;
}

void    uart_writeByte(uint8_t byteToWrite){
   U0TXBUF    =  byteToWrite;
}

uint8_t uart_readByte(void){
  return U0RXBUF;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr(void) {
   uart_clearTxInterrupts(); // TODO: do not clear, but disable when done
   uart_vars.txCb();
   return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr(void) {
   uart_clearRxInterrupts(); // TODO: do not clear, but disable when done
   uart_vars.rxCb();
   return DO_NOT_KICK_SCHEDULER;
}
