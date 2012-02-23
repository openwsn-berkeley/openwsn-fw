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
   cc2420_status_t     status;
   
   cc2420_FSCTRL_reg.FREQ         = 357;
   cc2420_FSCTRL_reg.LOCK_STATUS  = 0;
   cc2420_FSCTRL_reg.LOCK_LENGTH  = 0;
   cc2420_FSCTRL_reg.CAL_RUNNING  = 0;
   cc2420_FSCTRL_reg.CAL_DONE     = 0;
   cc2420_FSCTRL_reg.LOCK_THR     = 1;
   
   radio_spiWriteReg(CC2420_FSCTRL_ADDR, &status, *(uint16_t*)&cc2420_FSCTRL_reg);
}

void radio_rfOn() {
   cc2420_status_t status;
   
   radio_spiStrobe(CC2420_SXOSCON, &status);
   while (status.xosc16m_stable==0) {
      radio_spiStrobe(CC2420_SNOP, &status);
   }
}

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   cc2420_status_t status;
   
   radio_spiStrobe(CC2420_SFLUSHTX, &status);
   radio_spiWriteTxFifo(&status, packet, len);
}

void radio_txEnable() {
   cc2420_status_t status;
   
   radio_spiStrobe(CC2420_SRXON, &status);
   
   radio_spiStrobe(CC2420_SNOP, &status);
   while (status.rssi_valid==1) {
      radio_spiStrobe(CC2420_SNOP, &status);
   }
   /*
   cc2420_status_t status;
   
   radio_spiStrobe(CC2420_STXCAL, &status);
   
   while (status.lock==0) {
      radio_spiStrobe(CC2420_SNOP, &status);
   }
   */
}

void radio_txNow() {
   cc2420_status_t       status;
   cc2420_FSCTRL_reg_t   cc2420_FSCTRL_reg;
   cc2420_FSMSTATE_reg_t cc2420_FSMSTATE_reg;
   
   radio_spiStrobe(CC2420_STXON, &status);
   
   radio_spiStrobe(CC2420_SNOP, &status);
   while (status.tx_active==1) {
      radio_spiStrobe(CC2420_SNOP, &status);
   }
   
   radio_spiReadReg(CC2420_FSCTRL_ADDR,&status,(uint8_t*)&cc2420_FSCTRL_reg);
   cc2420_FSCTRL_reg = cc2420_FSCTRL_reg;
   
   radio_spiReadReg(CC2420_FSMSTATE_ADDR,&status,(uint8_t*)&cc2420_FSMSTATE_reg);
   cc2420_FSMSTATE_reg = cc2420_FSMSTATE_reg;
}

void radio_rxEnable() {
   // poipoi
}

void radio_rxNow() {
   // poipoi
}

void radio_getReceivedFrame(uint8_t* bufRead, uint8_t* lenRead, uint8_t maxBufLen) {
   cc2420_status_t status;
   
   radio_spiReadRxFifo(&status, bufRead, lenRead, maxBufLen);
}

void radio_rfOff() {
   // poipoi
}

//=========================== private =========================================

void radio_spiStrobe(uint8_t strobe, cc2420_status_t* statusRead) {
   uint8_t              spi_tx_buffer[1];
   uint8_t              spi_rx_buffer[1];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | strobe);
   spi_txrx(&(spi_tx_buffer[0]),sizeof(spi_tx_buffer),&(spi_rx_buffer[0]), 0);
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
}

void radio_spiWriteReg(uint8_t reg, cc2420_status_t* statusRead, uint16_t regValueToWrite) {
   uint8_t              spi_tx_buffer[3];
   uint8_t              spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | reg);
   spi_tx_buffer[1]     = regValueToWrite/256;
   spi_tx_buffer[2]     = regValueToWrite%256;
   spi_txrx(&(spi_tx_buffer[0]),sizeof(spi_tx_buffer),&(spi_rx_buffer[0]), 0);
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
}

void radio_spiReadReg(uint8_t reg, cc2420_status_t* statusRead, uint8_t* regValueRead) {
   uint8_t              spi_tx_buffer[3];
   uint8_t              spi_rx_buffer[3];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_READ | CC2420_FLAG_REG | reg);
   spi_tx_buffer[1]     = 0;
   spi_tx_buffer[2]     = 0;
   spi_txrx(&(spi_tx_buffer[0]),sizeof(spi_tx_buffer),&(spi_rx_buffer[0]), 0);
   
   *statusRead          = *(cc2420_status_t*)&spi_rx_buffer[0];
   *(regValueRead+0)    = spi_rx_buffer[2];
   *(regValueRead+1)    = spi_rx_buffer[1];
}

void radio_spiWriteTxFifo(cc2420_status_t* statusRead, uint8_t* bufToWrite, uint8_t len) {
   *(bufToWrite-1)      = (CC2420_FLAG_WRITE | CC2420_FLAG_REG | CC2420_TXFIFO_ADDR);
   
   spi_txrx(bufToWrite-1,len+3,(uint8_t*)statusRead,1);
}

void radio_spiReadRxFifo(cc2420_status_t* statusRead, uint8_t* bufRead, uint8_t* lenRead, uint8_t maxBufLen) {
   uint8_t              spi_tx_buffer[10];
   
   spi_tx_buffer[0]     = (CC2420_FLAG_READ | CC2420_FLAG_REG | CC2420_RXFIFO_ADDR);
   spi_tx_buffer[1]     = 0;
   spi_tx_buffer[2]     = 0;
   
   spi_txrx(&spi_tx_buffer[0],10,bufRead,0);
}

//=========================== interrupt handlers ==============================
