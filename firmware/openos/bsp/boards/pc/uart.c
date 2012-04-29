/**
\brief PC-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
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
   // poipoipoi stub
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
  // poipoipoi stub
}

void    uart_disableInterrupts(){
  // poipoipoi stub
}

void    uart_clearRxInterrupts(){
  // poipoipoi stub
}

void    uart_clearTxInterrupts(){
  // poipoipoi stub
}

void    uart_writeByte(uint8_t byteToWrite){
  // poipoipoi stub
}

uint8_t uart_readByte(){
  // poipoipoi stub
}

//=========================== interrupt handlers ==============================