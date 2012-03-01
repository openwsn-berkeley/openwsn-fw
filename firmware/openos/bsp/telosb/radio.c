/**
\brief TelosB-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "string.h"
#include "radio.h"
#include "cc2420.h"
#include "spi.h"
#include "radiotimer.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   cc2420_status_t radioStatusByte;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

void radio_spiStrobe     (uint8_t strobe, cc2420_status_t* statusRead);
void radio_spiWriteReg   (uint8_t reg,    cc2420_status_t* statusRead, uint16_t regValueToWrite);
void radio_spiReadReg    (uint8_t reg,    cc2420_status_t* statusRead, uint8_t* regValueRead);
void radio_spiWriteTxFifo(                cc2420_status_t* statusRead, uint8_t* bufToWrite, uint8_t  lenToWrite);
void radio_spiReadRxFifo (                cc2420_status_t* statusRead, uint8_t* bufRead,    uint8_t* lenRead, uint8_t maxBufLen);

//=========================== public ==========================================

void radio_init() {
   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // reset radio
   radio_reset();
   
   radiotimer_start(0xffff);//poipoi
}

void radio_startTimer(uint16_t period) {
   radiotimer_start(period);
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_setStartFrameCb(cb);
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   radiotimer_setEndFrameCb(cb);
}

void radio_reset() {
   uint16_t              delay;
   
   // VREG pin
   P4DIR     |=  0x20;                           // P4.5 radio VREG enabled, output
   P4OUT     |=  0x20;                           // P4.5 radio VREG enabled, hold high
   for (delay=0xffff;delay>0;delay--);           // max. VREG start-up time is 0.6ms
   
   // reset low
   P4DIR     |=  0x40;                           // P4.6 radio reset, output
   P4OUT     &= ~0x40;                           // P4.6 radio reset, hold low
   for (delay=0xffff;delay>0;delay--);
   
   // reset high
   P4OUT     |=  0x40;                           // P4.6 radio reset, hold high
   for (delay=0xffff;delay>0;delay--);
}

void radio_setFrequency(uint8_t frequency) {
   cc2420_FSCTRL_reg_t cc2420_FSCTRL_reg;
   
   cc2420_FSCTRL_reg.FREQ         = frequency-11;
   cc2420_FSCTRL_reg.FREQ        *= 5;
   cc2420_FSCTRL_reg.FREQ        += 357;
   cc2420_FSCTRL_reg.LOCK_STATUS  = 0;
   cc2420_FSCTRL_reg.LOCK_LENGTH  = 0;
   cc2420_FSCTRL_reg.CAL_RUNNING  = 0;
   cc2420_FSCTRL_reg.CAL_DONE     = 0;
   cc2420_FSCTRL_reg.LOCK_THR     = 1;
   
   radio_spiWriteReg(CC2420_FSCTRL_ADDR,
                     &radio_vars.radioStatusByte,
                     *(uint16_t*)&cc2420_FSCTRL_reg);
}

void radio_rfOn() {
   radio_spiStrobe(CC2420_SXOSCON, &radio_vars.radioStatusByte);
   while (radio_vars.radioStatusByte.xosc16m_stable==0) {
      radio_spiStrobe(CC2420_SNOP, &radio_vars.radioStatusByte);
   }
}

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   radio_spiStrobe(CC2420_SFLUSHTX, &radio_vars.radioStatusByte);
   radio_spiWriteTxFifo(&radio_vars.radioStatusByte, packet, len);
}

void radio_txEnable() {
   /*
   // I don't fully understand how the CC2420_STXCA the can be used.
   */
}

void radio_txNow() {
   radio_spiStrobe(CC2420_STXON, &radio_vars.radioStatusByte);
}

void radio_rxEnable() {
   // poipoi
}

void radio_rxNow() {
   // poipoi
}

void radio_getReceivedFrame(uint8_t* bufRead,
                            uint8_t* lenRead,
                            uint8_t  maxBufLen,
                                int* rssi,
                            uint8_t* lqi,
                            uint8_t* crc) {
   // poipoi
   radio_spiReadRxFifo(&radio_vars.radioStatusByte, bufRead, lenRead, maxBufLen);
}

void radio_rfOff() {
   // poipoi
}

//=========================== private =========================================

void radio_spiStrobe(uint8_t strobe, cc2420_status_t* statusRead) {
   uint8_t  spi_tx_buffer[1];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | strobe);
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_FIRSTBYTE,
            (uint8_t*)statusRead,
            1,
            SPI_FIRST,
            SPI_LAST);
}

void radio_spiWriteReg(uint8_t reg, cc2420_status_t* statusRead, uint16_t regValueToWrite) {
   uint8_t              spi_tx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | reg);
   spi_tx_buffer[1]     = regValueToWrite/256;
   spi_tx_buffer[2]     = regValueToWrite%256;
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_FIRSTBYTE,
            (uint8_t*)statusRead,
            1,
            SPI_FIRST,
            SPI_LAST);
}

void radio_spiReadReg(uint8_t reg, cc2420_status_t* statusRead, uint8_t* regValueRead) {
   uint8_t              spi_tx_buffer[3];
   uint8_t              spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_READ | CC2420_FLAG_REG | reg);
   spi_tx_buffer[1]     = 0x00;
   spi_tx_buffer[2]     = 0x00;
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_LAST);
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
   *(regValueRead+0)    = spi_rx_buffer[2];
   *(regValueRead+1)    = spi_rx_buffer[1];
}

void radio_spiWriteTxFifo(cc2420_status_t* statusRead, uint8_t* bufToWrite, uint8_t len) {
   uint8_t              spi_tx_buffer[2];
   
   // step 1. send SPI address and length byte
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | CC2420_TXFIFO_ADDR);
   spi_tx_buffer[1]     = len;
   
   spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_FIRSTBYTE,
            (uint8_t*)statusRead,
            1,
            SPI_FIRST,
            SPI_NOTLAST);
   
   // step 2. send payload
   spi_txrx(bufToWrite,
            len,
            SPI_LASTBYTE,
            (uint8_t*)statusRead,
            1,
            SPI_NOTFIRST,
            SPI_LAST);
}

void radio_spiReadRxFifo(cc2420_status_t* statusRead,
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
   spi_txrx(spi_tx_buffer,
            2,
            SPI_BUFFER,
            spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_NOTLAST);
   
   *pLenRead  = spi_rx_buffer[1];
   
   if (*pLenRead>2 && *pLenRead<=127) {
      // valid length
      
      //read packet
      spi_txrx(spi_tx_buffer,
               *pLenRead,
               SPI_BUFFER,
               pBufRead,
               125,
               SPI_NOTFIRST,
               SPI_LAST);
      
   } else {
      // invalid length
      
      // read a just byte to close spi
      spi_txrx(spi_tx_buffer,
               1,
               SPI_BUFFER,
               spi_rx_buffer,
               sizeof(spi_rx_buffer),
               SPI_NOTFIRST,
               SPI_LAST);
   }
}

//=========================== callbacks =======================================
