/**
\brief TelosB-specific definition of the "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "spi.h"
#include "leds.h"

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

void spi_init() {
   // clear variables
   memset(&spi_vars,0,sizeof(spi_vars_t));
   
   // hold USART state machine in reset mode during configuration
   U0CTL      =  SWRST;                          // [b0] SWRST=1: Enabled. USART logic held in reset state
   
   // configure SPI-related pins
   P3SEL     |=  0x02;                           // P3.1 in SIMO mode
   P3DIR     |=  0x02;                           // P3.1 as output
   P3SEL     |=  0x04;                           // P3.2 in SOMI mode
   P3DIR     |=  0x04;                           // P3.2 as output
   P3SEL     |=  0x08;                           // P3.3 in SCL mode
   P3DIR     |=  0x08;                           // P3.3 as output 
   P4OUT     |=  0x04;                           // P4.2 radio CS, hold high
   P4DIR     |=  0x04;                           // P4.2 radio CS, output
   
   // initialize USART registers
   U0CTL     |=  CHAR | SYNC | MM ;              // [b7]          0: unused
                                                 // [b6]          0: unused
                                                 // [b5]      I2C=0: SPI mode (not I2C)   
                                                 // [b4]     CHAR=1: 8-bit data
                                                 // [b3]   LISTEN=0: Disabled
                                                 // [b2]     SYNC=1: SPI mode (not UART)
                                                 // [b1]       MM=1: USART is master
                                                 // [b0]    SWRST=x: don't change
   
   U0TCTL     =  CKPH | SSEL1 | STC | TXEPT;     // [b7]     CKPH=1: UCLK is delayed by one half cycle
                                                 // [b6]     CKPL=0: normal clock polarity
                                                 // [b5]    SSEL1=1:
                                                 // [b4]    SSEL0=0: SMCLK
                                                 // [b3]          0: unused
                                                 // [b2]          0: unused
                                                 // [b1]      STC=1: 3-pin SPI mode
                                                 // [b0]    TXEPT=1: UxTXBUF and TX shift register are empty                                                 
   
   U0BR1      =  0x00;
   U0BR0      =  0x02;                           // U0BR = [U0BR1<<8|U0BR0] = 2
   U0MCTL     =  0x00;                           // no modulation needed in SPI mode
      
   // enable USART module
   ME1       |=  UTXE0 | URXE0;                  // [b7]    UTXE0=1: USART0 transmit enabled
                                                 // [b6]    URXE0=1: USART0 receive enabled
                                                 // [b5]          x: don't touch!
                                                 // [b4]          x: don't touch!
                                                 // [b3]          x: don't touch!
                                                 // [b2]          x: don't touch!
                                                 // [b1]          x: don't touch!
                                                 // [b0]          x: don't touch!
   
   // clear USART state machine from reset, starting operation
   U0CTL     &= ~SWRST;
   
   // enable interrupts via the IEx SFRs
#ifdef SPI_IN_INTERRUPT_MODE
   IE1       |=  URXIE0;                         // we only enable the SPI RX interrupt
                                                 // since TX and RX happen concurrently,
                                                 // i.e. an RX completion necessarily
                                                 // implies a TX completion.
#endif
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCb(spi_cbt cb) {
   spi_vars.spi_cb = cb;
}
#endif

void spi_txrx(uint8_t*     bufTx,
              uint8_t      lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint8_t      maxLenBufRx,
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
      P4OUT                 &= ~0x04;
   }
   
#ifdef SPI_IN_INTERRUPT_MODE
   // implementation 1. use a callback function when transaction finishes
   
   // write first byte to TX buffer
   U0TXBUF                   = *spi_vars.pNextTxByte;
   
   // re-enable interrupts
   __enable_interrupt();
#else
   // implementation 2. busy wait for each byte to be sent
   
   // send all bytes
   while (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
      U0TXBUF                = *spi_vars.pNextTxByte;
      // busy wait on the interrupt flag
      while ((IFG1 & URXIFG0)==0);
      // clear the interrupt flag
      IFG1                  &= ~URXIFG0;
      // save the byte just received in the RX buffer
      switch (spi_vars.returnType) {
         case SPI_FIRSTBYTE:
            if (spi_vars.numTxedBytes==0) {
               *spi_vars.pNextRxByte   = U0RXBUF;
            }
            break;
         case SPI_BUFFER:
            *spi_vars.pNextRxByte      = U0RXBUF;
            spi_vars.pNextRxByte++;
            break;
         case SPI_LASTBYTE:
            *spi_vars.pNextRxByte      = U0RXBUF;
            break;
      }
      // one byte less to go
      spi_vars.pNextTxByte++;
      spi_vars.numTxedBytes++;
      spi_vars.txBytesLeft--;
   }
   
   // put CS signal high to signal end of transmission to slave
   if (spi_vars.isLast==SPI_LAST) {
      P4OUT                 |=  0x04;
   }
   
   // SPI is not busy anymore
   spi_vars.busy             =  0;
#endif
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t spi_isr() {
#ifdef SPI_IN_INTERRUPT_MODE   
   // save the byte just received in the RX buffer
   switch (spi_vars.returnType) {
      case SPI_FIRSTBYTE:
         if (spi_vars.numTxedBytes==0) {
            *spi_vars.pNextRxByte   = U0RXBUF;
         }
         break;
      case SPI_BUFFER:
         *spi_vars.pNextRxByte      = U0RXBUF;
         spi_vars.pNextRxByte++;
         break;
      case SPI_LASTBYTE:
         *spi_vars.pNextRxByte      = U0RXBUF;
         break;
   }
   
   // one byte less to go
   spi_vars.pNextTxByte++;
   spi_vars.numTxedBytes++;
   spi_vars.txBytesLeft--;
   
   if (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
      U0TXBUF                = *spi_vars.pNextTxByte;
   } else {
      // put CS signal high to signal end of transmission to slave
      if (spi_vars.isLast==SPI_LAST) {
         P4OUT                 |=  0x04;
      }
      // SPI is not busy anymore
      spi_vars.busy             =  0;
      
      // SPI is done!
      if (spi_vars.callback!=NULL) {
         // call the callback
         spi_vars.spi_cb();
         // kick the OS
         return KICK_SCHEDULER;
      }
   }
   return DO_NOT_KICK_SCHEDULER;
#else
   // this should never happpen!
   
   // we can not print from within the BSP. Instead:
   // blink the error LED
   leds_error_blink();
   // reset the board
   board_reset();
   
   return DO_NOT_KICK_SCHEDULER; // we will not get here, statement to please compiler
#endif
}
