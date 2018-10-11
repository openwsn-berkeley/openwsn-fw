/**
\brief GINA-specific definition of the "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "spi.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   // information about the current transaction
   uint8_t*        pNextTxByte;
   uint8_t         numTxedBytes;
   uint8_t         txBytesLeft;
   spi_return_t    returnType;
   uint8_t*        pNextRxByte;
   uint8_t         maxRxBytes;
   spi_first_t     isFirst;
   spi_last_t      isLast;
   // state of the module
   uint8_t         busy;
#ifdef SPI_IN_INTERRUPT_MODE
   // callback when module done
   spi_cbt         callback;
#endif
} spi_vars_t;

spi_vars_t spi_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void spi_init(void) {
   // clear variables
   memset(&spi_vars,0,sizeof(spi_vars_t));
   
   // hold USART state machine in reset mode during configuration
   UCA0CTL1   =  UCSWRST;                        // [b0] SWRST=1: Enabled. USART logic held in reset state
   
   // configure SPI-related pins
   P3SEL     |=  0x31;                           // P3SEL = 0bxx11xxx1, MOSI/MISO/CLK pins
   P4OUT     |=  0x01;                           // EN_RF (P4.0) pin is SPI chip select, set high
   P4DIR     |=  0x01;                           // EN_RF (P4.0) pin as output
   P4OUT     |=  0x40;                           // set P4.6 pin high (mimicks EN_RF)
   P4DIR     |=  0x40;                           // P4.6 pin as output
   
   // initialize USART registers
   UCA0CTL1  |=  UCSSEL1 + UCSSEL0;               // SMCLK, reset
   UCA0CTL0   =  UCCKPH + UCMSB + UCMST + UCSYNC; // polarity, MSB first, master mode, 3-pin SPI
   UCA0BR0    =  0x03;                            // UCLK/4
   UCA0BR1    =  0x00;                            // 0
   
   // clear USART state machine from reset, starting operation
   UCA0CTL1  &= ~UCSWRST;
   
   // enable interrupts via the IEx SFRs
#ifdef SPI_IN_INTERRUPT_MODE
   IE2       |=  UCA0RXIE;                       // we only enable the SPI RX interrupt
                                                 // since TX and RX happen concurrently,
                                                 // i.e. an RX completion necessarily
                                                 // implies a TX completion.
#endif
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCallback(spi_cbt cb) {
   spi_vars.callback = cb;
}
#endif

void spi_txrx(uint8_t*     bufTx,
              uint16_t     lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint16_t     maxLenBufRx,
              spi_first_t  isFirst,
              spi_last_t   isLast) {

#ifdef SPI_IN_INTERRUPT_MODE
   // disable interrupts
   __disable_interrupt();
#endif
   
   // register spi frame to send
   spi_vars.pNextTxByte      =  bufTx;
   spi_vars.numTxedBytes     =  0;
   spi_vars.txBytesLeft      =  lenbufTx;
   spi_vars.returnType       =  returnType;
   spi_vars.pNextRxByte      =  bufRx;
   spi_vars.maxRxBytes       =  maxLenBufRx;
   spi_vars.isFirst          =  isFirst;
   spi_vars.isLast           =  isLast;
   
   // SPI is now busy
   spi_vars.busy             =  1;
   
   // lower CS signal to have slave listening
   if (spi_vars.isFirst==SPI_FIRST) {
      P4OUT                 &= ~0x01;
   }
   
#ifdef SPI_IN_INTERRUPT_MODE
   // implementation 1. use a callback function when transaction finishes
   
   // write first byte to TX buffer
   UCA0TXBUF                 = *spi_vars.pNextTxByte;
   
   // re-enable interrupts
   __enable_interrupt();
#else
   // implementation 2. busy wait for each byte to be sent
   
   // send all bytes
   while (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
      UCA0TXBUF              = *spi_vars.pNextTxByte;
      // busy wait on the interrupt flag
      while ((IFG2 & UCA0RXIFG)==0);
      // clear the interrupt flag
      IFG2                  &= ~UCA0RXIFG;
      // save the byte just received in the RX buffer
      switch (spi_vars.returnType) {
         case SPI_FIRSTBYTE:
            if (spi_vars.numTxedBytes==0) {
               *spi_vars.pNextRxByte   = UCA0RXBUF;
            }
            break;
         case SPI_BUFFER:
            *spi_vars.pNextRxByte      = UCA0RXBUF;
            spi_vars.pNextRxByte++;
            break;
         case SPI_LASTBYTE:
            *spi_vars.pNextRxByte      = UCA0RXBUF;
            break;
      }
      // one byte less to go
      spi_vars.pNextTxByte++;
      spi_vars.numTxedBytes++;
      spi_vars.txBytesLeft--;
   }
   
   // put CS signal high to signal end of transmission to slave
   if (spi_vars.isLast==SPI_LAST) {
      P4OUT                 |=  0x01;
   }
   
   // SPI is not busy anymore
   spi_vars.busy             =  0;
#endif
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t spi_isr(void) {
#ifdef SPI_IN_INTERRUPT_MODE
   // save the byte just received in the RX buffer
   switch (spi_vars.returnType) {
      case SPI_FIRSTBYTE:
         if (spi_vars.numTxedBytes==0) {
            *spi_vars.pNextRxByte = UCA0RXBUF;
         }
         break;
      case SPI_BUFFER:
         *spi_vars.pNextRxByte    = UCA0RXBUF;
         spi_vars.pNextRxByte++;
         break;
      case SPI_LASTBYTE:
         *spi_vars.pNextRxByte    = UCA0RXBUF;
         break;
   }
   
   // one byte less to go
   spi_vars.pNextTxByte++;
   spi_vars.numTxedBytes++;
   spi_vars.txBytesLeft--;
   
   if (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
      UCA0TXBUF              = *spi_vars.pNextTxByte;
   } else {
      // put CS signal high to signal end of transmission to slave
      if (spi_vars.isLast==SPI_LAST) {
         P4OUT              |=  0x01;
      }
      // SPI is not busy anymore
      spi_vars.busy          =  0;
      
      // SPI is done!
      if (spi_vars.callback!=NULL) {
         // call the callback
         spi_vars.callback();
         // kick the OS
         return KICK_SCHEDULER;
      }
   }
#else
   while(1);// this should never happen
#endif
}
