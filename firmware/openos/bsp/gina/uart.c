/**
\brief GINA-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
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
   uint8_t*        rxBufWrPtr;
   uint8_t*        rxBufRdPtr;
   uint8_t         rxBufFill;
   uart_rx_cbt     rx_cb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

void reset_rxBuf();

//=========================== public ==========================================

void uart_init() {
   P3SEL                    |=  0xC0;           // P3.6,7 = USCI_A1 TXD/RXD
   
   UCA1CTL1                 |=  UCSWRST;         // hold UART module in reset
   
#ifdef UART_BAUDRATE_115200
   //115200 baud, clocked from 4.8MHz SMCLK
   UCA1CTL1                 |=  UCSSEL_2;        // clocking from SMCLK
   UCA1BR0                   =  0x8a;            // 4.8MHz/115200 - 41.66
   UCA1BR1                   =  0x00;            //
   UCA1MCTL                  =  UCBRS_7;         // modulation
#else
   //9600 baud, clocked from 32kHz ACLK
   UTCTL1                   |=  SSEL0;           // clocking from ACLK
   UBR01                     =  0x03;            // 32768/9600 = 3.41
   UBR11                     =  0x00;            //
   UMCTL1                    =  0x4A;            // modulation
#endif
   
   UCA1CTL1                 &= ~UCSWRST;         // clear UART reset bit
}

//===== TX

void uart_txSetup(uart_txDone_cbt cb) {
   uart_vars.txDone_cb       = cb;               // register callback
}

void uart_tx(uint8_t* txBuf, uint8_t txBufLen) {
   
   // register data to send
   uart_vars.txBuf           = txBuf;
   uart_vars.txBufLen        = txBufLen;
   
   // enable UART TX interrupt
   UC1IFG                   &= ~UCA1TXIFG;
   UC1IE                    |=  UCA1TXIE;
   
   // send first byte
   UCA1TXBUF                 = *uart_vars.txBuf;
}

//===== RX

void uart_rxSetup(uint8_t*    rxBuf,
                  uint8_t     rxBufLen,
                  uint8_t     rxBufFillThres,
                  uart_rx_cbt cb) {
   uart_vars.rxBuf           = rxBuf;
   uart_vars.rxBufLen        = rxBufLen;
   uart_vars.rxBufFillThres  = rxBufFillThres;
   reset_rxBuf();
   uart_vars.rx_cb           = cb;
   
}

void uart_rxStart() {
   // enable UART RX interrupt
   UC1IFG                   &= ~UCA1RXIFG;
   UC1IE                    |=  UCA1RXIE;
}

void uart_readBytes(uint8_t* buf, uint8_t numBytes) {
   uint8_t i;
   
   for (i=0;i<numBytes;i++) {
      // copy byte into receive buffer
      *buf                   = *uart_vars.rxBufRdPtr;
      // advance counters
      buf++;
      uart_vars.rxBufRdPtr++;
      if (uart_vars.rxBufRdPtr>=uart_vars.rxBuf+uart_vars.rxBufLen) {
         uart_vars.rxBufRdPtr= uart_vars.rxBuf;
      }
   }
   
   // reduce fill
   uart_vars.rxBufFill      -= numBytes;
}

void uart_rxStop() {
   // disable UART1 RX interrupt
   UC1IE                    &= ~UCA1RXIE;
}

//=========================== private =========================================

void reset_rxBuf() {
   uart_vars.rxBufWrPtr      = uart_vars.rxBuf;
   uart_vars.rxBufRdPtr      = uart_vars.rxBuf;
   uart_vars.rxBufFill       = 0;
   
}

//=========================== interrupt handlers ==============================

#pragma vector = USCIAB1TX_VECTOR
__interrupt void USCIAB1TX_ISR (void) {
   // one byte less to go
   uart_vars.txBufLen--;
   uart_vars.txBuf++;
   
   if (uart_vars.txBufLen>0) {
      // send next byte
      UCA1TXBUF              = *uart_vars.txBuf;
   } else {
      if (uart_vars.txDone_cb!=NULL) {
         // disable UART1 TX interrupt
         UC1IE              &= ~UCA1TXIE;
         // call the callback
         uart_vars.txDone_cb();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
}

#pragma vector = USCIAB1RX_VECTOR
__interrupt void USCIAB1RX_ISR (void) {
   // copy received by into buffer
   *uart_vars.rxBufWrPtr     =  UCA1RXBUF;
   // shift pointer
   uart_vars.rxBufWrPtr++;
   if (uart_vars.rxBufWrPtr>=uart_vars.rxBuf+uart_vars.rxBufLen) {
      uart_vars.rxBufWrPtr   =  uart_vars.rxBuf;
   }
   // increment fill
   uart_vars.rxBufFill++;
   
   if        (uart_vars.rxBufFill>=uart_vars.rxBufLen) {
      // buffer has overflown
      
      // reset buffer
      reset_rxBuf();
      
      
      if (uart_vars.rx_cb!=NULL) {
         // call the callback
         uart_vars.rx_cb(UART_EVENT_OVERFLOW);
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
      }
      
   } else if (uart_vars.rxBufFill>=uart_vars.rxBufFillThres) {
      // buffer above threshold
      
      if (uart_vars.rx_cb!=NULL) {
         // call the callback
         uart_vars.rx_cb(UART_EVENT_THRES);
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
}