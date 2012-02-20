/**
\brief TelosB-specific definition of the "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "spi.h"

//=========================== defines =========================================

//#define ISR_SPI

//=========================== variables =======================================

typedef struct {
   uint8_t* tx_buffer;
   uint8_t* rx_buffer;
   uint8_t  num_bytes;
   uint8_t  busy;
} spi_vars_t;

spi_vars_t spi_vars;

//=========================== prototypes ======================================

void spi_txrx(uint8_t* spaceToSend, uint8_t len, uint8_t* spaceToReceive);

//=========================== public ==========================================

void spi_init() {
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
#ifdef ISR_SPI
   IE1       |=  URXIE0;                         // we only enable the SPI RX interrupt
                                                 // since TX and RX happen concurrently,
                                                 // i.e. an RX completion necessarily
                                                 // implies a TX completion.
#endif
}

#ifdef ISR_SPI
// implementation 1. use interrupts to signal that a byte was sent
void spi_txrx(uint8_t* spaceToSend, uint8_t len, uint8_t* spaceToReceive) {
   
   // disable interrupts
   __disable_interrupt();
   
   // register spi frame to send
   spi_vars.tx_buffer        =  spaceToSend;
   spi_vars.rx_buffer        =  spaceToReceive;
   spi_vars.num_bytes        =  len;
   spi_vars.busy             =  1;
   
   // lower CS signal to have slave listening
   P4OUT                    &= ~0x04;
   
   // send first byte
   U0TXBUF                   = *spi_vars.tx_buffer;
   spi_vars.tx_buffer++;
   spi_vars.num_bytes--;
   
   // re-enable interrupts
   __enable_interrupt();
}
#else
// implementation 1. busy wait for each byte to be sent
void spi_txrx(uint8_t* spaceToSend, uint8_t len, uint8_t* spaceToReceive) {
   
   //register spi frame to send
   spi_vars.tx_buffer        =  spaceToSend;
   spi_vars.rx_buffer        =  spaceToReceive;
   spi_vars.num_bytes        =  len;
   spi_vars.busy             =  1;
   
   // lower CS signal to have slave listening
   P4OUT                    &= ~0x04;
   
   // send all bytes
   while (spi_vars.num_bytes>0) {
      //write byte to TX buffer
      U0TXBUF                = *spi_vars.tx_buffer;
      spi_vars.tx_buffer++;
      // busy wait on the interrupt flag
      while ((IFG1 & URXIFG0)==0);
      // clear the interrupt flag
      IFG1                  &= ~URXIFG0;
      // save the byte just received in the RX buffer
      *spi_vars.rx_buffer    =  U0RXBUF;
      spi_vars.rx_buffer++;
      // one byte less to go
      spi_vars.num_bytes--;
   }
   
   // put CS signal high to signal end of transmission to slave
   P4OUT                    &= ~0x04;
}
#endif

//=========================== private =========================================

//=========================== interrupt handlers ==============================

#ifdef ISR_SPI
//executed in ISR, called from scheduler.c
void isr_spi_rx() {
   *spi_vars.rx_buffer = UCA0RXBUF;
   spi_vars.rx_buffer++;
   if (spi_vars.num_bytes>0) {
      UCA0TXBUF = *spi_vars.tx_buffer;
      spi_vars.tx_buffer++;
      spi_vars.num_bytes--;
   } else {
      P4OUT|=0x01;P4OUT|=0x40;                   // SPI CS (and P4.6) up
      spi_vars.busy = 0;
   }
}
#endif
