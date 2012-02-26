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

typedef void (*uart_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

void uart_init();
void uart_setTxDoneCb(uart_cbt cb);
void uart_setRxThresholdCb(uart_cbt cb);
void uart_tx(uint8_t* txBuf, uint8_t txBufLen);

#endif
