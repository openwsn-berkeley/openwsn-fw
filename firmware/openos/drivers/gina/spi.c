#include "msp430x26x.h"
#include "spi.h"
#include "radio.h"
#include "packetfunctions.h"

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
   UCA0CTL1  =  UCSSEL1 + UCSSEL0 + UCSWRST;     // SMCLK, reset
   UCA0CTL0  =  UCCKPH + UCMSB + UCMST + UCSYNC; // polarity, MSB first, master mode, 3-pin SPI
   UCA0BR0   =  0x03;                            // UCLK/4
   UCA0BR1   =  0x00;                            // 0
   P3SEL    |=  0x31;                            // P3SEL = 0bxx11xxx1, MOSI/MISO/CLK pins
   P4OUT    |=  0x01;                            // EN_RF (P4.0) pin is SPI chip select, set high
   P4DIR    |=  0x01;                            // EN_RF (P4.0) pin as output
   P4OUT    |=  0x40;                            // set P4.6 pin high (mimicks EN_RF)
   P4DIR    |=  0x40;                            // P4.6 pin as output
   UCA0CTL1 &= ~UCSWRST;                         // Initialize USART state machine
#ifdef ISR_SPI
   IE2      |=  UCA0RXIE;                        // Enable USCI_A0 RX/TX interrupt (TX and RX happen at the same time)
#endif
}

void spi_write_register(uint8_t reg_addr, uint8_t reg_setting) {
   uint8_t temp_tx_buffer[2];
   uint8_t temp_rx_buffer[2];
   temp_tx_buffer[0] = (0xC0 | reg_addr);        // turn addess in a 'reg write' address
   temp_tx_buffer[1] = reg_setting;
   spi_txrx(&(temp_tx_buffer[0]),sizeof(temp_tx_buffer),&(temp_rx_buffer[0]));
#ifdef ISR_SPI
   while ( spi_vars.busy==1 );
#endif
}

uint8_t spi_read_register(uint8_t reg_addr) {
   uint8_t temp_tx_buffer[2];
   uint8_t temp_rx_buffer[2];
   temp_tx_buffer[0] = (0x80 | reg_addr);        // turn addess in a 'reg read' address
   temp_tx_buffer[1] = 0x00;                     // send a no_operation command just to get the reg value
   spi_txrx(&(temp_tx_buffer[0]),sizeof(temp_tx_buffer),&(temp_rx_buffer[0]));
#ifdef ISR_SPI
   while ( spi_vars.busy==1 );
#endif
   return temp_rx_buffer[1];
}

void spi_write_buffer(OpenQueueEntry_t* packet) {
   uint8_t temp_rx_buffer[1+1+127];              // 1B SPI address, 1B length, max. 127B data
   spi_txrx(packet->payload,packet->length,&(temp_rx_buffer[0]));
#ifdef ISR_SPI
   while ( spi_vars.busy==1 );
#endif
}

void spi_read_buffer(OpenQueueEntry_t* packet, uint8_t length) {
   uint8_t temp_tx_buffer[1+1+127];              // 1B SPI address, 1B length, 127B data
   temp_tx_buffer[0] = 0x20;                     // spi address for 'read frame buffer'
   spi_txrx(&(temp_tx_buffer[0]),length,packet->payload);
#ifdef ISR_SPI
   while ( spi_vars.busy==1 );
#endif
}

//=========================== private =========================================

#ifdef ISR_SPI
// this implemetation uses interrupts to signal that a byte was sent
void spi_txrx(uint8_t* spaceToSend, uint8_t len, uint8_t* spaceToReceive) {
   __disable_interrupt();
   //register spi frame to send
   spi_vars.tx_buffer =  spaceToSend;
   spi_vars.rx_buffer =  spaceToReceive;
   spi_vars.num_bytes     =  len;
   spi_vars.busy      =  1;
   //send first byte
   P4OUT&=~0x01;P4OUT&=~0x40;                    // SPI CS (and P4.6) down
   UCA0TXBUF     = *spi_vars.tx_buffer;
   spi_vars.tx_buffer++;
   spi_vars.num_bytes--;
   __enable_interrupt();
}
#else
// this implemetation busy waits for each byte to be sent
void spi_txrx(uint8_t* spaceToSend, uint8_t len, uint8_t* spaceToReceive) {
   //register spi frame to send
   spi_vars.tx_buffer = spaceToSend;
   spi_vars.rx_buffer = spaceToReceive;
   spi_vars.num_bytes     = len;
   // SPI CS (and P4.6) down
   P4OUT&=~0x01;P4OUT&=~0x40;                    // SPI CS (and P4.6) down
   // write all bytes
   while (spi_vars.num_bytes>0) {
      //write byte to TX buffer
      UCA0TXBUF     = *spi_vars.tx_buffer;
      spi_vars.tx_buffer++;
      // busy wait on the interrupt flag
      while ((IFG2 & UCA0RXIFG)==0);
      // clear the interrupt flag
      IFG2 &= ~UCA0RXIFG;
      // save the byte just received in the RX buffer
      *spi_vars.rx_buffer = UCA0RXBUF;
      spi_vars.rx_buffer++;
      // one byte less to go
      spi_vars.num_bytes--;
   }
   // SPI CS (and P4.6) up
   P4OUT|=0x01;P4OUT|=0x40;
}
#endif

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
