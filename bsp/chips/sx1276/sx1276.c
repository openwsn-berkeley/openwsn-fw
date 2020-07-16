#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "sx1276.h"
#include "spi.h"
#include "def_spitxrx.h"


void sx1276_spiWriteReg(uint8_t reg, uint8_t regValueToWrite) {

	uint8_t spi_tx_buffer[2];
    uint8_t spi_rx_buffer[2];

    reg = reg | (1 << 7);
    spi_tx_buffer[0]     = reg ;
    spi_tx_buffer[1]     = regValueToWrite;


    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_FIRSTBYTE,              // returnType
        spi_rx_buffer,              // bufRx
        sizeof(spi_rx_buffer),      // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
}


uint8_t sx1276_spiReadReg(uint8_t reg) {

    uint8_t spi_tx_buffer[2];
    uint8_t spi_rx_buffer[2];
   
    reg = reg & (~(1 << 7));

    spi_tx_buffer[0]  = reg;

    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx_buffer,              // bufRx
        sizeof(spi_rx_buffer),      // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
    return spi_rx_buffer[1];
}

/*void sx1276_spiWriteFifo(uint8_t* bufToWrite, uint16_t len) {
    uint8_t              spi_tx_buffer[4];
    uint16_t             register_bbc_txfll;
    uint16_t             register_bbc_fbtxs;

    

    // step 1. send packet length.
    spi_tx_buffer[0]     = (FLAG_WRITE | (uint8_t)(register_bbc_txfll >> 8));
    spi_tx_buffer[1]     = (uint8_t)(register_bbc_txfll & 0xFF);
    spi_tx_buffer[2]     = (uint8_t)(len & 0xFF);       // low byte of packet-length -> TXFLL
    spi_tx_buffer[3]     = (uint8_t)(len >> 8) & 0x07;  // high byte of packet-length -> TXFLH

    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_BUFFER,                 // returnType
        NULL,                       // bufRx
        1,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );

    spi_tx_buffer[0]     = (FLAG_WRITE | (uint8_t)(register_bbc_fbtxs >> 8));
    spi_tx_buffer[1]     = (uint8_t)(register_bbc_fbtxs& 0xFF);

    // step 2. send the address to the Tx buffer.
    spi_txrx(
        spi_tx_buffer,             // bufTx
        2,                         // lenbufTx
        SPI_FIRSTBYTE,             // returnType
        NULL,                      // bufRx
        1,                         // maxLenBufRx
        SPI_FIRST,                 // isFirst
        SPI_NOTLAST                // isLast
    );

    // step 3. send the payload.
    spi_txrx(
        bufToWrite,                 // bufTx
        len - LENGTH_CRC,    // lenbufTx
        SPI_LASTBYTE,               // returnType
        NULL,                       // bufRx
        1,                          // maxLenBufRx
        SPI_NOTFIRST,               // isFirst
        SPI_LAST                    // isLast
    );

}


void sx1276_spiReadRxFifo( uint8_t* pBufRead, uint16_t* lenRead, uint8_t type, uint16_t maxBuffLength) {

    uint8_t spi_tx_buffer[4];
    uint8_t spi_rx_buffer[4];
    uint16_t length;
    uint16_t             register_bbc_rxfll;
    uint16_t             register_bbc_fbrxs;

    switch(type){
    case FREQ_SUGHZ:
        register_bbc_rxfll = RG_BBC0_RXFLL;
        register_bbc_fbrxs = BASE_ADDR_BBC0_FB0;
        break;
    case FREQ_24GHZ:
        register_bbc_rxfll = RG_BBC1_RXFLL;
        register_bbc_fbrxs = BASE_ADDR_BBC1_FB1;
        break;
    default:
        return;
    }

    // step 1 - read packet length from RG_BBC0_RXFLL/_RXFLH:
    spi_tx_buffer[0] = (FLAG_READ | (uint8_t)(register_bbc_rxfll >> 8));
    spi_tx_buffer[1] = (uint8_t)(register_bbc_rxfll & 0xFF);
    spi_tx_buffer[2] = 0x00;
    spi_tx_buffer[3] = 0x00;

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
    length = (uint16_t)spi_rx_buffer[2]             // RXFLL
                    | ((uint16_t)(spi_rx_buffer[3] & 0x07) << 8);  // RXFLH

    if (length<=maxBuffLength){
        spi_tx_buffer[0]    = (FLAG_READ | (uint8_t)(register_bbc_fbrxs/256));
        spi_tx_buffer[1]    = (uint8_t)(register_bbc_fbrxs%256);
        //read FIFO
        spi_txrx(
            spi_tx_buffer,              // bufTx
            2,                          // size of the address
            SPI_BUFFER,                 // returnType
            spi_rx_buffer,              // bufRx
            2,                          // maxLenBufRx
            SPI_FIRST,                  // isFirst
            SPI_NOTLAST                 // isLast
        );

            spi_txrx(
            NULL,                        // bufTx
            length,                     // lenbufTx
            SPI_BUFFER,                 // returnType
            pBufRead,                  // bufRx
            length,                     // maxLenBufRx
            SPI_NOTFIRST,               // isFirst
            SPI_LAST                    // isLast
        );
    }
    *(lenRead) = length;
}*/
