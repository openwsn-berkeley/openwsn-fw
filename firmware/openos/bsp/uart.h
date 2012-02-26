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
   UART_EVENT_THRES_REACHED  = 0,
   UART_EVENT_OVERFLOW       = 0,
} uart_event_t;

typedef void (*uart_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

void uart_init();
// TX
void uart_txSetup(uart_cbt cb);
void uart_tx(uint8_t* txBuf, uint8_t txBufLen);
// RX
void uart_rxSetup(uint8_t* rxBuf,
                  uint8_t  rxBufLen,
                  uint8_t  rxBufFillThres,
                  uart_cbt cb);
void uart_rxStart();
void uart_rxStop();

#endif
