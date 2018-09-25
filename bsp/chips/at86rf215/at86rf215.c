/**
\brief AT86RF215-specific library.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, August 2016.
\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, December 2017.

*/

#include "board.h"
#include "at86rf215.h"
#include "spi.h"
#include "debugpins.h"
#include "leds.h"
#include "radio.h"


void at86rf215_spiStrobe(uint8_t strobe) {
    uint8_t  spi_tx_buffer[3];

    spi_tx_buffer[0]     = ((FLAG_WRITE) | (uint8_t)(RG_RF09_CMD/256));
    spi_tx_buffer[1]     = (uint8_t)(RG_RF09_CMD%256);
    spi_tx_buffer[2]     = strobe;

    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_FIRSTBYTE,              // returnType
        NULL,                       // bufRx
        1,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
}

void at86rf215_spiWriteReg(uint16_t reg, uint8_t regValueToWrite) {

    uint8_t spi_rx_buffer[3];
    uint8_t spi_tx_buffer[3];

    spi_tx_buffer[0]     = (FLAG_WRITE | (uint8_t)((reg)/256));
    spi_tx_buffer[1]     = (uint8_t)((reg)%256);
    spi_tx_buffer[2]     = regValueToWrite;

    spi_txrx(
        spi_tx_buffer,              // bufTx
        3,                          // lenbufTx
        SPI_FIRSTBYTE,              // returnType
        spi_rx_buffer,              // bufRx
        3,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
}

uint8_t at86rf215_spiReadReg(uint16_t regAddr16) {

    uint8_t              spi_tx_buffer[3];
    uint8_t              spi_rx_buffer[3];

    spi_tx_buffer[0] = (FLAG_READ | (uint8_t)(regAddr16 >> 8));
    spi_tx_buffer[1] = (uint8_t)(regAddr16 & 0xFF);
    spi_tx_buffer[2] = 0x00;

    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx_buffer,              // bufRx
        sizeof(spi_rx_buffer),      // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
    return spi_rx_buffer[2];
}

void at86rf215_spiWriteFifo(uint8_t* bufToWrite, uint16_t len) {
    uint8_t              spi_tx_buffer[4];

    // step 1. send packet length.
    spi_tx_buffer[0]     = (FLAG_WRITE | (uint8_t)(RG_BBC0_TXFLL >> 8));
    spi_tx_buffer[1]     = (uint8_t)(RG_BBC0_TXFLL & 0xFF);
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

    spi_tx_buffer[0]     = (FLAG_WRITE | (uint8_t)(RG_BBC0_FBTXS >> 8));
    spi_tx_buffer[1]     = (uint8_t)(RG_BBC0_FBTXS& 0xFF);

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

void at86rf215_spiReadRxFifo( uint8_t* pBufRead, uint16_t* lenRead) {

    uint8_t spi_tx_buffer[4];
    uint8_t spi_rx_buffer[4];
    uint16_t length;

    // step 1 - read packet length from RG_BBC0_RXFLL/_RXFLH:
    spi_tx_buffer[0] = (FLAG_READ | (uint8_t)(RG_BBC0_RXFLL >> 8));
    spi_tx_buffer[1] = (uint8_t)(RG_BBC0_RXFLL & 0xFF);
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

    spi_tx_buffer[0]    = (FLAG_READ | (uint8_t)(BASE_ADDR_BBC0_FB0/256));
    spi_tx_buffer[1]    = (uint8_t)(BASE_ADDR_BBC0_FB0%256);
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
    *(lenRead) = length;
}

uint8_t at86rf215_status (void){
    uint8_t spi_tx_buffer[3];
    uint8_t spi_rx_buffer[3];

    spi_tx_buffer[0] = (FLAG_READ | (uint8_t)(RG_RF09_STATE/256));
    spi_tx_buffer[1] = (uint8_t)(RG_RF09_STATE%256);
    spi_tx_buffer[2] = 0x00;

    spi_txrx(
        spi_tx_buffer,              // bufTx
        3,                          // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx_buffer,              // bufRx
        3,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );

    return spi_rx_buffer[2];
}

void at86rf215_read_isr (uint8_t* rf09_isr){
    uint8_t spi_tx[6];
    uint8_t spi_rx[6];

    memset(&spi_tx[0],0,sizeof(spi_tx));
    memset(&spi_rx[0],0,sizeof(spi_rx));
    spi_tx[0] = (FLAG_READ | (uint8_t)(RG_RF09_IRQS >> 8));
    spi_tx[1] = (uint8_t)(RG_RF09_IRQS & 0xFF);

    spi_txrx(
        spi_tx,                     // bufTx
        6,                          // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx,                      // bufRx
        6,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
    *(rf09_isr)     = spi_rx[2];
    *(rf09_isr+1)   = spi_rx[3];
    *(rf09_isr+2)   = spi_rx[4];
    *(rf09_isr+3)   = spi_rx[5];
}



void at86rf215_readBurst(uint16_t reg, uint8_t* regValueRead, uint16_t size){
    uint8_t spi_tx[2049];

    memset(&spi_tx[0],0,sizeof(spi_tx));

    spi_tx[0] = (uint8_t)(reg/256);
    spi_tx[1] = (uint8_t)(reg%256);

    spi_txrx(
        spi_tx,                     // bufTx
        sizeof(spi_tx),             // lenbufTx
        SPI_BUFFER,                 // returnType
        regValueRead,               // bufRx
        sizeof(spi_tx),             // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );

}

