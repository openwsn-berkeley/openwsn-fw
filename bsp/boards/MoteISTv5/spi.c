/**
\brief MoteISTv5-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
\author Diogo Guerra <diogoguerra@ist.utl.pt>, <dy090.guerra@gmail.com>, July 2015.
*/

#include "hal_MoteISTv5.h"
#include "spi.h"
#include "leds.h"

//=========================== defines =========================================

// CS:  P10.0 chip select High
#define CS_SEL     P10SEL
#define CS_DIR     P10DIR
#define CS_OUT     P10OUT
#define CS_PIN     0x01

// MOSI: P10.1
#define MOSI_SEL   P10SEL
#define MOSI_DIR   P10DIR
#define MOSI_OUT   P10OUT
#define MOSI_PIN   0x02

// MISO: P10.2
#define MISO_SEL   P10SEL
#define MISO_DIR   P10DIR
#define MISO_OUT   P10OUT
#define MISO_PIN   0x04

// SCK: P10.3
#define SCK_SEL    P10SEL
#define SCK_DIR    P10DIR
#define SCK_OUT    P10OUT
#define SCK_PIN    0x08



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
   spi_cbt         spi_cb;
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
  //MOSI_DIR  |=  MOSI_PIN;                       // MOSI output
  MISO_SEL  |=  MISO_PIN;                       // MISO mode
  //MISO_DIR  &= ~MISO_PIN;                       // MISO output
  SCK_SEL   |=  SCK_PIN;                        // SCK  mode
  //SCK_DIR   |=  SCK_PIN;                        // SCK  output
  CS_SEL    &= ~CS_PIN;                         // CS   GPI/O
  CS_OUT    |=  CS_PIN;                         // CS   hold high
  CS_DIR    |=  CS_PIN;                         // CS   output

  // hold USART state machine in reset mode during configuration
  UCB3CTL1 |= UCSWRST;                          // [b0] SWRST=1: Enabled. USART logic held in reset state

  // initialize USART registers
  UCB3CTL0 |= UCMST | UCMSB | UCMODE0 | UCSYNC; // MSB first | active low | Clock High
  UCB3CTL0 |= UCCKPH; //Change rising edge, Clock inactive Low !!!!!!!Data e clokada na subida e mudada na descida
  UCB3CTL1 |= UCSSEL_2;                   // Select SMCLK

  UCB3BR0 = 0x03;                         // /3 ~8,3(3)Mhz => CC2420 SPI MAX CLKFREQ = 10MHz
  UCB3BR1 = 0x00;                         //
    
  UCB3CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**

  // enable interrupts via the IEx SFRs
  #ifdef SPI_IN_INTERRUPT_MODE
  UCB3IE     |=  UCRXIE;                       // we only enable the SPI RX interrupt
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
    CS_OUT                &= ~CS_PIN;
  }

  #ifdef SPI_IN_INTERRUPT_MODE
  // implementation 1. use a callback function when transaction finishes

  // write first byte to TX buffer
  UCB3TXBUF                   = *spi_vars.pNextTxByte;

  // re-enable interrupts
  __enable_interrupt();
  #else
  // implementation 2. busy wait for each byte to be sent

  // send all bytes
  while (spi_vars.txBytesLeft>0) {
    // write next byte to TX buffer
    UCB3TXBUF                = *spi_vars.pNextTxByte;
    // busy wait on the interrupt flag
    while ((UCB3IFG & UCRXIFG)==0);
    // clear the interrupt flag
    UCB3IFG                  &= ~UCRXIFG;
    // save the byte just received in the RX buffer
    switch (spi_vars.returnType) {
       case SPI_FIRSTBYTE:
          if (spi_vars.numTxedBytes==0) {
             *spi_vars.pNextRxByte   = UCB3RXBUF;
          }
          break;
       case SPI_BUFFER:
          *spi_vars.pNextRxByte      = UCB3RXBUF;
          spi_vars.pNextRxByte++;
          break;
       case SPI_LASTBYTE:
          *spi_vars.pNextRxByte      = UCB3RXBUF;
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
          *spi_vars.pNextRxByte      = UCB3RXBUF;
       }
       break;
    case SPI_BUFFER:
       *spi_vars.pNextRxByte         = UCB3RXBUF;
       spi_vars.pNextRxByte++;
       break;
    case SPI_LASTBYTE:
       *spi_vars.pNextRxByte         = UCB3RXBUF;
       break;
 }
 
 // one byte less to go
 spi_vars.pNextTxByte++;
 spi_vars.numTxedBytes++;
 spi_vars.txBytesLeft--;
 
 if (spi_vars.txBytesLeft>0) {
    // write next byte to TX buffer
    UCB3TXBUF                     = *spi_vars.pNextTxByte;
 } else {
    // put CS signal high to signal end of transmission to slave
    if (spi_vars.isLast==SPI_LAST) {
       CS_OUT                  |=  CS_PIN;
    }
    // SPI is not busy anymore
    spi_vars.busy               =  0;
    
    // SPI is done!
    if (spi_vars.spi_cb!=NULL) {
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
