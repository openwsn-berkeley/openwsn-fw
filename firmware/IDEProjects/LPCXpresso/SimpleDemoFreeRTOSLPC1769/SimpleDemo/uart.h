/**
\brief Cross-platform declaration "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __UART_H
#define __UART_H

#include "stdint.h"
 
//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void uart_init();
void uart_tx(uint8_t c);

#endif
