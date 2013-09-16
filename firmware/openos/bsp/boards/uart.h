#ifndef __UART_H
#define __UART_H

/**
\addtogroup BSP
\{
\addtogroup uart
\{

\brief Cross-platform declaration "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
#include "board.h"
 
//=========================== define ==========================================

//=========================== typedef =========================================

typedef enum {
   UART_EVENT_THRES,
   UART_EVENT_OVERFLOW,
} uart_event_t;

typedef void (*uart_tx_cbt)();
typedef void (*uart_rx_cbt)();

//=========================== variables =======================================

//=========================== prototypes ======================================

void    uart_init();
void    uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb);
void    uart_enableInterrupts();
void    uart_disableInterrupts();
void    uart_clearRxInterrupts();
void    uart_clearTxInterrupts();
void    uart_writeByte(uint8_t byteToWrite);
uint8_t uart_readByte();
//void    uart_setFlag(bool flag);
//bool    uart_getFlag();

// interrupt handlers
kick_scheduler_t uart_tx_isr();
kick_scheduler_t uart_rx_isr();

/**
\}
\}
*/

#endif
