/**
\brief wsn430v14-specific definition of the "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "spi.h"
#include "leds.h"

//=========================== defines =========================================


// MOSI: P5.1
#define MOSI_SEL   P5SEL
#define MOSI_DIR   P5DIR
#define MOSI_OUT   P5OUT
#define MOSI_PIN   0x02

// MISO: P5.2
#define MISO_SEL   P5SEL
#define MISO_DIR   P5DIR
#define MISO_OUT   P5OUT
#define MISO_PIN   0x04

// SCK: P5.3
#define SCK_SEL    P5SEL
#define SCK_DIR    P5DIR
#define SCK_OUT    P5OUT
#define SCK_PIN    0x08

// CS: P4.2
#define CS_SEL     P4SEL
#define CS_DIR     P4DIR
#define CS_OUT     P4OUT
#define CS_PIN     0x04

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
   
   // configure SPI-related pins
   MOSI_SEL  |=  MOSI_PIN;                       // MOSI mode
   MOSI_DIR  |=  MOSI_PIN;                       // MOSI output
   MISO_SEL  |=  MISO_PIN;                       // MISO mode
   MISO_DIR  &= ~MISO_PIN;                       // MISO output
   SCK_SEL   |=  SCK_PIN;                        // SCK  mode
   SCK_DIR   |=  SCK_PIN;                        // SCK  output 
   CS_OUT    |=  CS_PIN;                         // CS   hold high
   CS_DIR    |=  CS_PIN;                         // CS   output
   
   // hold USART state machine in reset mode during configuration
   U1CTL      =  SWRST;                          // [b0] SWRST=1: Enabled. USART logic held in reset state
   
   // initialize USART registers
   U1CTL      =  CHAR | SYNC | MM | SWRST;       // [b7]          0: unused
                                                 // [b6]          0: unused
                                                 // [b5]      I2C=0: SPI mode (not I2C)   
                                                 // [b4]     CHAR=1: 8-bit data
                                                 // [b3]   LISTEN=0: Disabled
                                                 // [b2]     SYNC=1: SPI mode (not UART)
                                                 // [b1]       MM=1: USART is master
                                                 // [b0]    SWRST=x: don't change
   
   U1TCTL     =  CKPH | SSEL1 | STC;             // [b7]     CKPH=1: UCLK is delayed by one half cycle
                                                 // [b6]     CKPL=0: normal clock polarity
                                                 // [b5]    SSEL1=1:
                                                 // [b4]    SSEL0=0: SMCLK
                                                 // [b3]          0: unused
                                                 // [b2]          0: unused
                                                 // [b1]      STC=1: 3-pin SPI mode
                                                 // [b0]    TXEPT=1: UxTXBUF and TX shift register are empty
   
   U1RCTL     =  0x00;                           // clear errors
   U1BR0      =  0x02;                           // U1BR = [U1BR1<<8|U0BR0] = 2
   U1BR1      =  0x00;
   U1MCTL     =  0x00;                           // no modulation needed in SPI mode
      
   // enable USART module
   ME2       |=  USPIE1;                         // enable USART1 SPI
   
   // clear USART state machine from reset, starting operation
   U1CTL     &= ~SWRST;
   
   // enable interrupts via the IEx SFRs
#ifdef SPI_IN_INTERRUPT_MODE
   IE2       |=  URXIE1;                         // we only enable the SPI RX interrupt
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
      CS_OUT                &= ~CS_PIN;
   }
   
#ifdef SPI_IN_INTERRUPT_MODE
   // implementation 1. use a callback function when transaction finishes
   
   // write first byte to TX buffer
   U1TXBUF                   = *spi_vars.pNextTxByte;
   
   // re-enable interrupts
   __enable_interrupt();
#else
   // implementation 2. busy wait for each byte to be sent
   
   // send all bytes
   while (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
      U1TXBUF                = *spi_vars.pNextTxByte;
      // busy wait on the interrupt flag
      while ((IFG2 & URXIFG1)==0);
      // clear the interrupt flag
      IFG2                  &= ~URXIFG1;
      // save the byte just received in the RX buffer
      switch (spi_vars.returnType) {
         case SPI_FIRSTBYTE:
            if (spi_vars.numTxedBytes==0) {
               *spi_vars.pNextRxByte   = U1RXBUF;
            }
            break;
         case SPI_BUFFER:
            *spi_vars.pNextRxByte      = U1RXBUF;
            spi_vars.pNextRxByte++;
            break;
         case SPI_LASTBYTE:
            *spi_vars.pNextRxByte      = U1RXBUF;
            break;
      }
      // one byte less to go
      spi_vars.pNextTxByte++;
      spi_vars.numTxedBytes++;
      spi_vars.txBytesLeft--;
   }
   
   // put CS signal high to signal end of transmission to slave
   if (spi_vars.isLast==SPI_LAST) {
      CS_OUT                |=  CS_PIN;
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
            *spi_vars.pNextRxByte      = U1RXBUF;
         }
         break;
      case SPI_BUFFER:
         *spi_vars.pNextRxByte         = U1RXBUF;
         spi_vars.pNextRxByte++;
         break;
      case SPI_LASTBYTE:
         *spi_vars.pNextRxByte         = U1RXBUF;
         break;
   }
   
   // one byte less to go
   spi_vars.pNextTxByte++;
   spi_vars.numTxedBytes++;
   spi_vars.txBytesLeft--;
   
   if (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
      U1TXBUF                     = *spi_vars.pNextTxByte;
   } else {
      // put CS signal high to signal end of transmission to slave
      if (spi_vars.isLast==SPI_LAST) {
         CS_OUT                  |=  CS_PIN;
      }
      // SPI is not busy anymore
      spi_vars.busy               =  0;
      
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
   // this should never happen!
   
   // we can not print from within the BSP. Instead:
   // blink the error LED
   leds_error_blink();
   // reset the board
   board_reset();
   
   return DO_NOT_KICK_SCHEDULER; // we will not get here, statement to please compiler
#endif
}
