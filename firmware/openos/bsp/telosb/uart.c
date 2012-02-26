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
   uint8_t*        txBuf;
   uint8_t         txBufLen;
   uart_txDone_cbt txDone_cb;
   // RX
   uint8_t*        rxBuf;
   uint8_t         rxBufLen;
   uint8_t         rxBufFillThres;
   uint8_t*        rxBufPtr;
   uint8_t         rxBufFill;
   uart_rx_cbt     rx_cb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init() {
   P3SEL                     =  0xC0;            // P3.6,7 = UART1TX/RX
   
   UCTL1                     =  SWRST;           // hold UART1 module in reset
   UCTL1                    |=  CHAR;            // 8-bit character
   
#ifdef UART_BAUDRATE_115200
   //115200 baud, clocked from 4.8MHz SMCLK
   UTCTL1                   |=  SSEL1;           // clocking from SMCLK
   UBR01                     =  41;              // 4.8MHz/115200 - 41.66
   UBR11                     =  0x00;            //
   UMCTL1                    =  0x4A;            // modulation
#else
   //9600 baud, clocked from 32kHz ACLK
   UTCTL1                   |=  SSEL0;           // clocking from ACLK
   UBR01                     =  0x03;            // 32768/9600 = 3.41
   UBR11                     =  0x00;            //
   UMCTL1                    =  0x4A;            // modulation
#endif
   
   ME2                      |=  UTXE1 + URXE1;   // enable UART1 TX/RX
   UCTL1                    &= ~SWRST;           // clear UART1 reset bit
}

//===== TX

void uart_txSetup(uart_txDone_cbt cb) {
   uart_vars.txDone_cb       = cb;               // register callback
}

void uart_tx(uint8_t* txBuf, uint8_t txBufLen) {
   
   // register data to send
   uart_vars.txBuf           = txBuf;
   uart_vars.txBufLen        = txBufLen;
   
   // enable UART1 TX interrupt
   IFG2                     &= ~UTXIFG1;
   IE2                      |=  UTXIE1;
   
   // send first byte
   U1TXBUF                   = *uart_vars.txBuf;
}

//===== RX

void uart_rxSetup(uint8_t*    rxBuf,
                  uint8_t     rxBufLen,
                  uint8_t     rxBufFillThres,
                  uart_rx_cbt cb) {
   uart_vars.rxBuf           = rxBuf;
   uart_vars.rxBufLen        = rxBufLen;
   uart_vars.rxBufFillThres  = rxBufFillThres;
   uart_vars.rxBufPtr        = uart_vars.rxBuf ;
   uart_vars.rxBufFill       = 0;
   uart_vars.rx_cb           = cb;
   
}

void uart_rxStart() {
   // enable UART1 RX interrupt
   IFG2                     &= ~URXIFG1;
   IE2                      |=  URXIE1;
}

void uart_rxStop() {
   // disable UART1 RX interrupt
   IE2                      &= ~URXIE1;
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
      U1TXBUF                = *uart_vars.txBuf;
   } else {
      if (uart_vars.txDone_cb!=NULL) {
         // disable UART1 TX interrupt
         IE2                &= ~UTXIE1;
         // call the callback
         uart_vars.txDone_cb();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
}

#pragma vector = USART1RX_VECTOR
__interrupt void usart1rx_VECTOR (void) {
   // copy received by into buffer
   *uart_vars.rxBufPtr       = URXIE1;
   // shift pointer
   uart_vars.rxBufPtr++;
   if (uart_vars.rxBufPtr>uart_vars.rxBuf+uart_vars.rxBufLen) {
      uart_vars.rxBufPtr     = uart_vars.rxBuf;
   }
   // increment fill
   uart_vars.rxBufFill++;
   // call the callback
   if        (uart_vars.rxBufFill>=uart_vars.rxBufLen) {
      // buffer has overflown
      
      // reset buffer
      uart_vars.rxBufPtr        = uart_vars.rxBuf ;
      uart_vars.rxBufFill       = 0;
      
      // call the callback
      if (uart_vars.rx_cb!=NULL) {
         uart_vars.rx_cb(UART_EVENT_OVERFLOW);
      }
      // make sure CPU restarts after leaving interrupt
      __bic_SR_register_on_exit(CPUOFF);
      
   } else if (uart_vars.rxBufFill>=uart_vars.rxBufFillThres) {
      // buffer above threshold
      
      // call the callback
      if (uart_vars.rx_cb!=NULL) {
         uart_vars.rx_cb(UART_EVENT_THRES);
      }
      // make sure CPU restarts after leaving interrupt
      __bic_SR_register_on_exit(CPUOFF);
   }
}
