/**
\brief TelosB-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "radio.h"
#include "spi.h"
#include "cc2420.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void radio_spiStrobe     (uint8_t strobe, cc2420_status_t* statusRead);
void radio_spiWriteReg   (uint8_t reg,    cc2420_status_t* statusRead, uint16_t regValueToWrite);
void radio_spiReadReg    (uint8_t reg,    cc2420_status_t* statusRead, uint8_t* regValueRead);
void radio_spiWriteTxFifo(                cc2420_status_t* statusRead, uint8_t* bufToWrite, uint8_t  lenToWrite);
void radio_spiReadRxFifo (                cc2420_status_t* statusRead, uint8_t* bufRead,    uint8_t* lenRead, uint8_t maxBufLen);

//=========================== public ==========================================

void radio_init() {
   radio_reset();
}

void radio_reset() {
   uint16_t              delay;
   cc2420_MDMCTRL0_reg_t cc2420_MDMCTRL0_reg;
   cc2420_status_t       cc2420_status;
   
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
   // poipoi
}

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   
   
}

void radio_rfOn() {
   // poipoi
}

void radio_txEnable() {
   // poipoi
}

void radio_txNow() {
   // poipoi
}

void radio_rxEnable() {
   // poipoi
}

void radio_rxNow() {
   // poipoi
}

void radio_getReceivedFrame() {
   // poipoi
}

void radio_rfOff() {
   // poipoi
}

//=========================== private =========================================

void radio_spiStrobe(uint8_t strobe, cc2420_status_t* statusRead) {
   uint8_t              spi_tx_buffer[1];
   uint8_t              spi_rx_buffer[1];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | strobe);
   spi_txrx(&(spi_tx_buffer[0]),sizeof(spi_tx_buffer),&(spi_rx_buffer[0]));
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
}

void radio_spiWriteReg(uint8_t reg, cc2420_status_t* statusRead, uint16_t regValueToWrite) {
   uint8_t              spi_tx_buffer[3];
   uint8_t              spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | reg);
   spi_tx_buffer[1]     = regValueToWrite/256;
   spi_tx_buffer[2]     = regValueToWrite%256;
   spi_txrx(&(spi_tx_buffer[0]),sizeof(spi_tx_buffer),&(spi_rx_buffer[0]));
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
}

void radio_spiReadReg(uint8_t reg, cc2420_status_t* statusRead, uint8_t* regValueRead) {
   uint8_t              spi_tx_buffer[3];
   uint8_t              spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_READ | CC2420_FLAG_REG | reg);
   spi_tx_buffer[1]     = 0;
   spi_tx_buffer[2]     = 0;
   spi_txrx(&(spi_tx_buffer[0]),sizeof(spi_tx_buffer),&(spi_rx_buffer[0]));
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
   *(regValueRead+0)    = spi_rx_buffer[2];
   *(regValueRead+1)    = spi_rx_buffer[1];
}

void radio_spiWriteTxFifo(cc2420_status_t* statusRead, uint8_t* bufToWrite, uint8_t len) {
}

void radio_spiReadRxFifo(cc2420_status_t* statusRead, uint8_t* bufRead, uint8_t* lenRead, uint8_t maxBufLen) {
}

//=========================== interrupt handlers ==============================
