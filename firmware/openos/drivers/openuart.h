#ifndef __UART_H
#define __UART_H

#include "openwsn.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void openuart_init();
void openuart_clearInt();
void openuart_txByte(uint8_t byteToTx);
void openuart_clearIntOnly();
void openuart_clearTxFlag();
void openuart_clearRxFlag();
uint8_t openuart_getRxByte();

#endif
