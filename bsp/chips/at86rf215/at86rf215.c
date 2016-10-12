/**
\brief CC1200-specific library.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, August 2016.
*/

#include "board.h"
#include "radio.h"
#include "at86rf215.h"
#include "spi.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "leds.h"

//extern const cc1200_rf_cfg_t cc1200_rf_cfg;

void at86rf215_spiStrobe(uint8_t strobe, /*radio_status_t* statusRead,*/ uint16_t address) {
   uint8_t  spi_tx_buffer[3];
   
   spi_tx_buffer[0]     = (FLAG_WRITE | (uint8_t)(address/256));
   spi_tx_buffer[1]     = (uint8_t)(address%256);
   spi_tx_buffer[0]     = strobe;

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

void at86rf215_spiWriteReg(uint16_t reg, /*radio_status_t* statusRead,*/ uint8_t regValueToWrite) {
     
   
   uint8_t spi_tx_buffer[3];
  
   tempAddr  = (uint8_t)(reg & 0x00FF);
        
   spi_tx_buffer[0]     = (FLAG_WRITE | (uint8_t)((reg)/256));
   spi_tx_buffer[1]     = (uint8_t)((reg)%256);
   spi_tx_buffer[2]     = regValueToWrite;
      
   spi_txrx(
       spi_tx_buffer,              // bufTx
       3,                          // lenbufTx
       SPI_FIRSTBYTE,              // returnType
       (uint8_t*)statusRead,       // bufRx
       1,                          // maxLenBufRx
       SPI_FIRST,                  // isFirst
       SPI_LAST                    // isLast
   );
}

void at86rf215_spiReadReg(uint16_t reg, /*radio_status_t* statusRead,*/ uint8_t* regValueRead) {
  
    uint8_t              spi_tx_buffer[3];
    uint8_t              spi_rx_buffer[3];
   
    spi_tx_buffer[0]     = (FLAG_READ | (uint8_t)((reg)/256));
    spi_tx_buffer[1]     = (uint8_t)((reg)%256);
    spi_tx_buffer[2]     = 0x00;    

   
    spi_txrx(
        spi_tx_buffer,              // bufTx
        3,                          // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx_buffer,              // bufRx
        sizeof(spi_rx_buffer),      // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
    //*statusRead          = *(radio_status_t*)&spi_rx_buffer[0];
    *(regValueRead+0)    = spi_rx_buffer[2];
   
}

void at86rf215_spiWriteFifo(/*radio_status_t* statusRead,*/ uint8_t* bufToWrite, uint16_t len) {
    uint8_t              spi_tx_buffer[4];
    uint8_t              spi_rx_buffer[length];  
    // step 1. send packet length. 
    spi_tx_buffer[0]     = (radio_FLAG_WRITE | 0x03);
    spi_tx_buffer[1]     = 0x06;
    spi_tx_buffer[2]     = (radio_FLAG_READ | (uint8_t)((len)/256));
    spi_tx_buffer[3]     = (uint8_t)((len)%256);   
  
    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_FIRSTBYTE,              // returnType
        (uint8_t*)statusRead,       // bufRx
        1,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );

    spi_tx_buffer[0]     = (FLAG_WRITE | 0xa8);
    spi_tx_buffer[1]     = 0x00;
   
    // step 2. send the address to the Tx buffer.
    spi_txrx(
        bufToWrite,                // bufTx
        2,                         // lenbufTx
        SPI_FIRSTBYTE,             // returnType
        (uint8_t*)statusRead,      // bufRx
        1,                         // maxLenBufRx
        SPI_FIRST,                 // isFirst
        SPI_NOTLAST                // isLast
    );

    // step 3. send the payload.
    spi_txrx(
        spi_tx_buffer,             // bufTx
        len,                       // lenbufTx
        SPI_LASTBYTE,              // returnType
        spi_rx_buffer,             // bufRx
        sizeof(spi_rx_buffer),     // maxLenBufRx
        SPI_NOTFIRST,              // isFirst
        SPI_LAST                   // isLast
    );
}

void at86rf215_spiReadRxFifo(radio_status_t* statusRead,
                         uint8_t*         pBufRead,
                         /*uint8_t*         pLenRead,
                         uint8_t          maxBufLen*/) {
    // when reading the packet over SPI from the RX buffer, you get the following:
    // - *[1B]     dummy byte because of SPI
    // - *[1B]     length byte
    // -  [0-125B] packet (excluding CRC)
    // - *[2B]     CRC
    uint8_t spi_tx_buffer[4];
    uint8_t spi_rx_buffer[4];
    uint8_t PHR[2];
    uint16_t length;
     
    spi_tx_buffer[0]    = (FLAG_READ | 0x03);
    spi_tx_buffer[1]    = 0x04;
    spi_tx_buffer[2]    = 0x00;
    spi_tx_buffer[3]    = 0x00;  

    //read FIFO length
    spi_txrx(
        spi_tx_buffer,              // bufTx
        4,                          // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx_buffer,              // bufRx
        4,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                   // isLast
    );
    PHR[0] = spi_rx_buffer[2];
    PHR[1] = spi_rx_buffer[3];
    
    length = PHR[1] + (PHR[0] & (0x07))*256; 
    
    spi_tx_buffer[0]    = (FLAG_READ | 0x20);
    spi_tx_buffer[1]    = 0x00;      
    //read FIFO 
    spi_txrx(
        spi_tx_buffer,              // bufTx
        length,                     // lenbufTx
        SPI_BUFFER,                 // returnType
        pBufRead,                  // bufRx
        length,                     // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );

}

//const cc1200_rf_cfg_t cc1200_rf_cfg = {
//  .register_settings = cc1200_register_settings,
//  .size_of_register_settings = sizeof(cc1200_register_settings),
//  .chan_center_freq0 = RF_CFG_CHAN_CENTER_F0,
//  .chan_spacing = RF_CFG_CHAN_SPACING,
//  .min_channel = RF_CFG_MIN_CHANNEL,
//  .max_channel = RF_CFG_MAX_CHANNEL,
//  .min_txpower = RF_CFG_MIN_TXPOWER,
//  .max_txpower = RF_CFG_MAX_TXPOWER,
//  .cca_threshold = RF_CFG_CCA_THRESHOLD,
//};

