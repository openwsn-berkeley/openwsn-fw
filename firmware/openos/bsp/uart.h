/**
\brief Cross-platform declaration "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __UART_H
#define __UART_H

#include "stdint.h"
 
//=========================== define ==========================================

//#define UART_BAUDRATE_115200

//=========================== typedef =========================================

typedef enum {
   UART_EVENT_THRES,
   UART_EVENT_OVERFLOW,
} uart_event_t;

typedef void (*uart_txDone_cbt)(void);
typedef void (*uart_rx_cbt)(uart_event_t ev);

//=========================== variables =======================================

//=========================== prototypes ======================================

void uart_init();
// TX
void uart_txSetup(uart_txDone_cbt cb);
void uart_tx(uint8_t* txBuf, uint8_t txBufLen);
// RX
void uart_rxSetup(uint8_t*    rxBuf,
                  uint8_t     rxBufLen,
                  uint8_t     rxBufFillThres,
                  uart_rx_cbt cb);
void uart_rxStart();
void uart_readBytes(uint8_t* buf, uint8_t numBytes);
void uart_rxStop();

#endif
