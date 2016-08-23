/**
\brief CC2420-specific library.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "board.h"
#include "cc2420.h"
#include "spi.h"


void cc2420_spiStrobe(uint8_t strobe, cc2420_status_t* statusRead) {
   uint8_t  spi_tx_buffer[1];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | strobe);
   
   spi_txrx(
      spi_tx_buffer,              // bufTx
      sizeof(spi_tx_buffer),      // lenbufTx
      SPI_FIRSTBYTE,              // returnType
      (uint8_t*)statusRead,       // bufRx
      1,                          // maxLenBufRx
      SPI_FIRST,                  // isFirst
      SPI_LAST                    // isLast
   );
}

void cc2420_spiWriteReg(uint8_t reg, cc2420_status_t* statusRead, uint16_t regValueToWrite) {
   uint8_t              spi_tx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | reg);
   spi_tx_buffer[1]     = regValueToWrite/256;
   spi_tx_buffer[2]     = regValueToWrite%256;
   
   spi_txrx(
      spi_tx_buffer,              // bufTx
      sizeof(spi_tx_buffer),      // lenbufTx
      SPI_FIRSTBYTE,              // returnType
      (uint8_t*)statusRead,       // bufRx
      1,                          // maxLenBufRx
      SPI_FIRST,                  // isFirst
      SPI_LAST                    // isLast
   );
}

void cc2420_spiReadReg(uint8_t reg, cc2420_status_t* statusRead, uint8_t* regValueRead) {
   uint8_t              spi_tx_buffer[3];
   uint8_t              spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_READ | CC2420_FLAG_REG | reg);
   spi_tx_buffer[1]     = 0x00;
   spi_tx_buffer[2]     = 0x00;
   
   spi_txrx(
      spi_tx_buffer,              // bufTx
      sizeof(spi_tx_buffer),      // lenbufTx
      SPI_BUFFER,                 // returnType
      spi_rx_buffer,              // bufRx
      sizeof(spi_rx_buffer),      // maxLenBufRx
      SPI_FIRST,                  // isFirst
      SPI_LAST                    // isLast
   );
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
   *(regValueRead+0)    = spi_rx_buffer[2];
   *(regValueRead+1)    = spi_rx_buffer[1];
}

void cc2420_spiWriteFifo(cc2420_status_t* statusRead, uint8_t* bufToWrite, uint8_t len, uint8_t addr) {
   uint8_t              spi_tx_buffer[2];
   
   // step 1. send SPI address and length byte
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | addr);
   spi_tx_buffer[1]     = len;
   
   spi_txrx(
      spi_tx_buffer,              // bufTx
      sizeof(spi_tx_buffer),      // lenbufTx
      SPI_FIRSTBYTE,              // returnType
      (uint8_t*)statusRead,       // bufRx
      1,                          // maxLenBufRx
      SPI_FIRST,                  // isFirst
      SPI_NOTLAST                 // isLast
   );
   
   // step 2. send payload
   spi_txrx(
      bufToWrite,                 // bufTx
      len,                        // lenbufTx
      SPI_LASTBYTE,               // returnType
      (uint8_t*)statusRead,       // bufRx
      1,                          // maxLenBufRx
      SPI_NOTFIRST,               // isFirst
      SPI_LAST                    // isLast
   );
}

void cc2420_spiReadRxFifo(cc2420_status_t* statusRead,
                         uint8_t*         pBufRead,
                         uint8_t*         pLenRead,
                         uint8_t          maxBufLen) {
   // when reading the packet over SPI from the RX buffer, you get the following:
   // - *[1B]     dummy byte because of SPI
   // - *[1B]     length byte
   // -  [0-125B] packet (excluding CRC)
   // - *[2B]     CRC
   uint8_t spi_tx_buffer[125];
   uint8_t spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_READ | CC2420_FLAG_REG | CC2420_RXFIFO_ADDR);
   
   // 2 first bytes
   spi_txrx(
      spi_tx_buffer,              // bufTx
      2,                          // lenbufTx
      SPI_BUFFER,                 // returnType
      spi_rx_buffer,              // bufRx
      sizeof(spi_rx_buffer),      // maxLenBufRx
      SPI_FIRST,                  // isFirst
      SPI_NOTLAST                 // isLast
   );
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
   *pLenRead            = spi_rx_buffer[1];
   
   if (*pLenRead>2 && *pLenRead<=127) {
      // valid length
      
      //read packet
      spi_txrx(
         spi_tx_buffer,           // bufTx
         *pLenRead,               // lenbufTx
         SPI_BUFFER,              // returnType
         pBufRead,                // bufRx
         125,                     // maxLenBufRx
         SPI_NOTFIRST,            // isFirst
         SPI_LAST                 // isLast
      );
      
   } else {
      // invalid length
      
      // read a just byte to close spi
      spi_txrx(
         spi_tx_buffer,           // bufTx
         1,                       // lenbufTx
         SPI_BUFFER,              // returnType
         spi_rx_buffer,           // bufRx
         sizeof(spi_rx_buffer),   // maxLenBufRx
         SPI_NOTFIRST,            // isFirst
         SPI_LAST                 // isLast
      );
   }
   /*
   A SFLUSHRX command strobe is required 
   after an RXFIFO overflow to enable 
   reception of new data. Note that the 
   SFLUSHRX command strobe should be 
   issued twice to ensure that the SFD pin 
   goes back to its inactive state.
   */
   
   cc2420_spiStrobe(CC2420_SFLUSHRX, statusRead);
   cc2420_spiStrobe(CC2420_SFLUSHRX, statusRead);
}

void cc2420_spiReadRam(uint16_t addr,
                         cc2420_status_t* statusRead,
                         uint8_t*         pBufRead,
                         uint8_t          len) {

   uint8_t spi_tx_buffer[2];
   uint8_t spi_rx_buffer[3];

// step 1. send SPI address
   spi_tx_buffer[0] = (CC2420_FLAG_RAM | (addr & 0x7F));
   spi_tx_buffer[1] = ((addr >> 1) & 0xC0) | CC2420_FLAG_RAM_READ;
 
   // 2 first bytes
   spi_txrx(
      spi_tx_buffer,              // bufTx
      2,                          // lenbufTx
      SPI_BUFFER,                 // returnType
      spi_rx_buffer,              // bufRx
      sizeof(spi_rx_buffer),      // maxLenBufRx
      SPI_FIRST,                  // isFirst
      SPI_NOTLAST                 // isLast
   );
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
   
   //read the actual bytes in RAM
   spi_txrx(
      spi_tx_buffer,           // bufTx
      len,                     // lenbufTx   // we will transfer len-2 garbage bytes, while receiving. does it matter?
      SPI_BUFFER,              // returnType
      pBufRead,                // bufRx
      len,                     // maxLenBufRx
      SPI_NOTFIRST,            // isFirst
      SPI_LAST                 // isLast
   );
}

void cc2420_spiWriteRam(uint16_t addr,
                        cc2420_status_t* statusRead,
                        uint8_t* bufToWrite,
                        uint8_t len) {
   uint8_t spi_tx_buffer[2];

// step 1. send SPI address
   spi_tx_buffer[0] = (CC2420_FLAG_RAM | (addr & 0x7F));
   spi_tx_buffer[1] = ((addr >> 1) & 0xC0) | CC2420_FLAG_RAM_WRITE;
   
   spi_txrx(
      spi_tx_buffer,              // bufTx
      sizeof(spi_tx_buffer),      // lenbufTx
      SPI_FIRSTBYTE,              // returnType
      (uint8_t*)statusRead,       // bufRx
      1,                          // maxLenBufRx
      SPI_FIRST,                  // isFirst
      SPI_NOTLAST                 // isLast
   );
   
   // step 2. send payload
   spi_txrx(
      bufToWrite,                 // bufTx
      len,                        // lenbufTx
      SPI_LASTBYTE,               // returnType
      (uint8_t*)statusRead,       // bufRx
      1,                          // maxLenBufRx
      SPI_NOTFIRST,               // isFirst
      SPI_LAST                    // isLast
   );
}

