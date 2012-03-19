/**
\brief Cross-platform declaration "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __UART_H
#define __UART_H

#include "stdint.h"
 
//=========================== define ==========================================

#define UART_BAUDRATE_115200

//=========================== typedef =========================================

typedef enum {
   UART_EVENT_THRES,
   UART_EVENT_OVERFLOW,
} uart_event_t;

typedef void (*uart_txDone_cbt)(void);
typedef void (*uart_rx_cbt)(uart_event_t ev);

//=========================== variables =======================================

//=========================== prototypes ======================================

void    uart_init();

//added by fabien:
void    uart_enableInterrupts();
void    uart_disableInterrupts();
void    uart_clearRxInterrupts();
void    uart_clearTxInterrupts();
void    uart_writeByte(uint8_t byteToWrite);
uint8_t uart_readByte();

uint8_t uart_isr_tx();
uint8_t uart_isr_rx();


#endif