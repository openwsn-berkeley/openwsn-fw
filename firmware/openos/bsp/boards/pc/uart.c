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
   printf("TODO uart_init\r\n");
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
  // poipoipoi stub
  printf("TODO uart_enableInterrupts\r\n");
}

void    uart_disableInterrupts(){
  // poipoipoi stub
  printf("TODO uart_disableInterrupts\r\n");
}

void    uart_clearRxInterrupts(){
  // poipoipoi stub
  printf("TODO uart_clearRxInterrupts\r\n");
}

void    uart_clearTxInterrupts(){
  // poipoipoi stub
  printf("TODO uart_clearTxInterrupts\r\n");
}

void    uart_writeByte(uint8_t byteToWrite){
  // poipoipoi stub
  printf("TODO uart_writeByte\r\n");
}

uint8_t uart_readByte(){
  // poipoipoi stub
  printf("TODO uart_readByte\r\n");
}

//=========================== interrupt handlers ==============================