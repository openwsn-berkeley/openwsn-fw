/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:06.140591.
*/
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
#include "board_obj.h"
 
//=========================== define ==========================================

//=========================== typedef =========================================

typedef enum {
   UART_EVENT_THRES,
   UART_EVENT_OVERFLOW,
} uart_event_t;

typedef void (*uart_tx_cbt)(OpenMote* self);
typedef void (*uart_rx_cbt)(OpenMote* self);

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void uart_init(OpenMote* self);
void uart_setCallbacks(OpenMote* self, uart_tx_cbt txCb, uart_rx_cbt rxCb);
void uart_enableInterrupts(OpenMote* self);
void uart_disableInterrupts(OpenMote* self);
void uart_clearRxInterrupts(OpenMote* self);
void uart_clearTxInterrupts(OpenMote* self);
void uart_writeByte(OpenMote* self, uint8_t byteToWrite);
uint8_t uart_readByte(OpenMote* self);

// interrupt handlers
kick_scheduler_t uart_tx_isr(OpenMote* self);
kick_scheduler_t uart_rx_isr(OpenMote* self);

/**
\}
\}
*/

#endif
