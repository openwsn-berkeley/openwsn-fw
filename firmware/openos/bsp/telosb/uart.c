/**
\brief TelosB-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "uart.h"

//=========================== defines =========================================

#define BAUDRATE_115200

//=========================== variables =======================================

typedef struct {
   uint8_t callBack;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init() {
   P3SEL      =  0xC0;                           // P3.6,7 = UART1TX/RX
   
   ME2       |=  UTXE1 + URXE1;                  // enable UART1 TX/RX
   UCTL1     |=  CHAR;                           // 8-bit character
   
#ifdef BAUDRATE_115200
   //115200 baud, clocked from 4.8MHz SMCLK
   UTCTL1    |=  SSEL1;                          // clocking from SMCLK
   UBR01      =  41;                             // 4.8MHz/115200 - 41.66
   UBR11      =  0x00;                           //
   UMCTL1     =  0x4A;                           // modulation
#else
   //9600 baud, clocked from 32kHz ACLK
   UTCTL1    |=  SSEL0;                          // clocking from ACLK
   UBR01      =  0x03;                           // 32768/9600 = 3.41
   UBR11      =  0x00;                           //
   UMCTL1     =  0x4A;                           // modulation
#endif
   
   UCTL1     &= ~SWRST;                          // clear UART1 reset bit
   //IE2       |=  URXIE1;                         // enable UART1 RX interrupt
}

void uart_tx(uint8_t c) {
   U1TXBUF    = c;
}

//=========================== private =========================================