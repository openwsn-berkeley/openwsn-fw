/**
\brief CC1200-specific library.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, August 2016.
*/

#include "board.h"
#include "CC1200.h"
#include "spi.h"

extern const cc1200_rf_cfg_t cc1200_rf_cfg;

void CC1200_spiStrobe(uint8_t strobe, cc1200_status_t* statusRead) {
   uint8_t  spi_tx_buffer[1];
   
   spi_tx_buffer[0]     = (CC1200_FLAG_WRITE | strobe);
   
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

void CC1200_spiWriteReg(uint16_t reg, cc1200_status_t* statusRead, uint8_t regValueToWrite) {
   
   uint8_t tempExt  = (uint8_t)(reg>>8);
   uint8_t tempAddr = (uint8_t)(reg & 0x00FF);
   
     /* Decide what register space is accessed */  
   if(!tempExt)
   {
   uint8_t              spi_tx_buffer[2];
   spi_tx_buffer[0]     = (CC1200_FLAG_WRITE | tempAddr);
   spi_tx_buffer[1]     = regValueToWrite;
      
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
   else if (tempExt == 0x2F)
   {
   uint8_t              spi_tx_buffer[3];
   spi_tx_buffer[0]     = (CC1200_FLAG_WRITE | tempExt);
   spi_tx_buffer[1]     = tempAddr;
   spi_tx_buffer[2]     = regValueToWrite;
      
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
  
}

void CC1200_spiReadReg(uint16_t reg, cc1200_status_t* statusRead, uint8_t* regValueRead) {
  
    uint8_t              spi_tx_buffer[3];
    uint8_t              spi_rx_buffer[3];
    uint8_t tempExt  = (uint8_t)(reg>>8);
    uint8_t tempAddr = (uint8_t)(reg & 0x00FF);
   
    if(!tempExt)
    {

    spi_tx_buffer[0]     = (CC1200_FLAG_READ | tempAddr);
    spi_tx_buffer[1]     = 0x00;
   
    spi_txrx(
        spi_tx_buffer,              // bufTx
        2,                          // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx_buffer,              // bufRx
        sizeof(spi_rx_buffer),      // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
    *statusRead          = *(cc1200_status_t*)&spi_rx_buffer[0];
    *(regValueRead+0)    = spi_rx_buffer[1];
    }
    else if (tempExt == 0x2F)
    {

    spi_tx_buffer[0]     = (CC1200_FLAG_READ | tempExt);
    spi_tx_buffer[1]     = tempAddr;
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
    *statusRead          = *(cc1200_status_t*)&spi_rx_buffer[0];
    *(regValueRead+0)    = spi_rx_buffer[2];
    *(regValueRead+1)    = spi_rx_buffer[1];
    }
   
}

void CC1200_spiWriteFifo(cc1200_status_t* statusRead, uint8_t* bufToWrite, uint8_t len, uint8_t addr) {
   uint8_t              spi_tx_buffer[1];
   
   // step 1. send SPI address and length byte
   spi_tx_buffer[0]     = ((CC1200_FLAG_WRITE | CC1200_FLAG_WRITE_BURST )| addr);

   
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

void CC1200_spiReadRxFifo(cc1200_status_t* statusRead,
                         uint8_t*         pBufRead,
                         uint8_t*         pLenRead,
                         uint8_t          maxBufLen) {
   // when reading the packet over SPI from the RX buffer, you get the following:
   // - *[1B]     dummy byte because of SPI
   // - *[1B]     length byte
   // -  [0-125B] packet (excluding CRC)
   // - *[2B]     CRC
   uint8_t spi_tx_buffer[125];
   //uint8_t spi_rx_buffer[127];
   
   spi_tx_buffer[0]     = (/*CC1200_FLAG_READ*/CC1200_FLAG_READ_BURST | CC1200_FIFO_ADDR);
      //read FIFO length 
   
   // 2 first bytes
   spi_txrx(
      spi_tx_buffer,              // bufTx
      *pLenRead,                   // lenbufTx
      SPI_BUFFER,                 // returnType
      pBufRead/*spi_rx_buffer*/,              // bufRx
      127/*sizeof(spi_rx_buffer)*/,      // maxLenBufRx
      SPI_FIRST,                  // isFirst
      SPI_LAST/*SPI_NOTLAST*/                 // isLast
   );
   
 //  *statusRead          = *(cc1200_status_t*)&spi_rx_buffer[0];
 //  *pLenRead            = spi_rx_buffer[1];
   
 //  if (*pLenRead>2 && *pLenRead<=127) {
      // valid length
      
      //read packet
  /*    spi_txrx(
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
   } */
   /*
   A SFRX command strobe is required 
   after an RXFIFO overflow to enable 
   reception of new data. Note that the 
   SFLUSHRX command strobe should be 
   issued twice to ensure that the SFD pin 
   goes back to its inactive state.
   */
   
   CC1200_spiStrobe(CC1200_SFRX, statusRead);
}

const cc1200_rf_cfg_t cc1200_rf_cfg = {
  .register_settings = cc1200_register_settings,
  .size_of_register_settings = sizeof(cc1200_register_settings),
  .chan_center_freq0 = RF_CFG_CHAN_CENTER_F0,
  .chan_spacing = RF_CFG_CHAN_SPACING,
  .min_channel = RF_CFG_MIN_CHANNEL,
  .max_channel = RF_CFG_MAX_CHANNEL,
  .min_txpower = RF_CFG_MIN_TXPOWER,
  .max_txpower = RF_CFG_MAX_TXPOWER,
  .cca_threshold = RF_CFG_CCA_THRESHOLD,
};

