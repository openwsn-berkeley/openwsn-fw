/**
\brief TelosB-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "stdio.h"
#include "uart.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   // TX
   uint8_t*   txBuf;
   uint8_t    txBufLen;
   uart_cbt   txDone_cb;
   // RX
   uint8_t*   rxBuffer;
   uint8_t    rxBytesReceived;
   uint8_t    rxBytesTreshold;
   uart_cbt   rxThreshold_cb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init() {
   P3SEL      =  0xC0;                           // P3.6,7 = UART1TX/RX
   
   UCTL1      =  SWRST;                          // hold UART1 module in reset
   UCTL1     |=  CHAR;                           // 8-bit character
   
#ifdef UART_BAUDRATE_115200
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
   
   ME2       |=  UTXE1 + URXE1;                  // enable UART1 TX/RX
   UCTL1     &= ~SWRST;                          // clear UART1 reset bit
   IE2       |=  UTXIE1 + URXIE1;                // enable UART1 TX/RX interrupt
}

void uart_setTxDoneCb(uart_cbt cb) {
   uart_vars.txDone_cb       = cb;
}

void uart_setRxThresholdCb(uart_cbt cb) {
   uart_vars.rxThreshold_cb  = cb;
}

void uart_tx(uint8_t* txBuf, uint8_t txBufLen) {
   
   // register data to send
   uart_vars.txBuf           = txBuf;
   uart_vars.txBufLen        = txBufLen;
   
   // send first byte
   U1TXBUF                   = *uart_vars.txBuf;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

#pragma vector = USART1TX_VECTOR
__interrupt void usart1tx_VECTOR (void) {
   // one byte less to go
   uart_vars.txBufLen--;
   uart_vars.txBuf++;
   
   if (uart_vars.txBufLen>0) {
      // send next byte
      U1TXBUF                   = *uart_vars.txBuf;
   } else {
      if (uart_vars.txDone_cb!=NULL) {
         uart_vars.txDone_cb();
      }
   }
}

#pragma vector = USART1RX_VECTOR
__interrupt void usart1rx_VECTOR (void) {
   P5OUT    ^=  0x10;                           // toggle LED
}
