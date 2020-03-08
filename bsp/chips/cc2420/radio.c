/**
\brief CC2420-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "board.h"
#include "radio.h"
#include "cc2420.h"
#include "spi.h"
#include "debugpins.h"
#include "leds.h"
#include "sctimer.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   cc2420_status_t radioStatusByte;
   radio_state_t   state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== public ==========================================

//===== admin

void radio_init(void) {
   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // change state
   radio_vars.state          = RADIOSTATE_STOPPED;
   
   // reset radio
   radio_reset();
   
   // change state
   radio_vars.state          = RADIOSTATE_RFOFF;
   
}

void radio_setStartFrameCb(radio_capture_cbt cb) {
  sctimer_setStartFrameCb(cb);
}

void radio_setEndFrameCb(radio_capture_cbt cb) {
   sctimer_setEndFrameCb(cb);
}

//===== reset

void radio_reset(void) {
   volatile uint16_t     delay;
   cc2420_MDMCTRL0_reg_t cc2420_MDMCTRL0_reg;
   cc2420_TXCTRL_reg_t   cc2420_TXCTRL_reg;
   cc2420_RXCTRL1_reg_t  cc2420_RXCTRL1_reg;
   
   // set radio VREG pin high
   PORT_PIN_RADIO_VREG_HIGH();
   for (delay=0xffff;delay>0;delay--);           // max. VREG start-up time is 0.6ms
   
   // set radio RESET pin low
   PORT_PIN_RADIO_RESET_LOW();
   for (delay=0xffff;delay>0;delay--);
   
   // set radio RESET pin high
   PORT_PIN_RADIO_RESET_HIGH();
   for (delay=0xffff;delay>0;delay--);
   
   // disable address recognition
   cc2420_MDMCTRL0_reg.PREAMBLE_LENGTH      = 2; // 3 leading zero's (IEEE802.15.4 compliant)
   cc2420_MDMCTRL0_reg.AUTOACK              = 0;
   cc2420_MDMCTRL0_reg.AUTOCRC              = 1;
   cc2420_MDMCTRL0_reg.CCA_MODE             = 3;
   cc2420_MDMCTRL0_reg.CCA_HYST             = 2;
   cc2420_MDMCTRL0_reg.ADR_DECODE           = 0; // turn OFF address recognition
   cc2420_MDMCTRL0_reg.PAN_COORDINATOR      = 0;
   cc2420_MDMCTRL0_reg.RESERVED_FRAME_MODE  = 1; // accept all frame types
   cc2420_MDMCTRL0_reg.reserved_w0          = 0;
   cc2420_spiWriteReg(
      CC2420_MDMCTRL0_ADDR,
      &radio_vars.radioStatusByte,
      *(uint16_t*)&cc2420_MDMCTRL0_reg
   );
   
   // speed up time to TX
   cc2420_TXCTRL_reg.PA_LEVEL               = 31;// max. TX power (~0dBm)
   cc2420_TXCTRL_reg.reserved_w1            = 1;
   cc2420_TXCTRL_reg.PA_CURRENT             = 3;
   cc2420_TXCTRL_reg.TXMIX_CURRENT          = 0;
   cc2420_TXCTRL_reg.TXMIX_CAP_ARRAY        = 0;
   cc2420_TXCTRL_reg.TX_TURNAROUND          = 0; // faster STXON->SFD timing (128us)
   cc2420_TXCTRL_reg.TXMIXBUF_CUR           = 2;
   cc2420_spiWriteReg(
      CC2420_TXCTRL_ADDR,
      &radio_vars.radioStatusByte,
      *(uint16_t*)&cc2420_TXCTRL_reg
   );
   
   // apply correction recommended in datasheet
   cc2420_RXCTRL1_reg.RXMIX_CURRENT         = 2;
   cc2420_RXCTRL1_reg.RXMIX_VCM             = 1;
   cc2420_RXCTRL1_reg.RXMIX_TAIL            = 1;
   cc2420_RXCTRL1_reg.LNA_CAP_ARRAY         = 1;
   cc2420_RXCTRL1_reg.MED_HGM               = 0;
   cc2420_RXCTRL1_reg.HIGH_HGM              = 1;
   cc2420_RXCTRL1_reg.MED_LOWGAIN           = 0;
   cc2420_RXCTRL1_reg.LOW_LOWGAIN           = 1;
   cc2420_RXCTRL1_reg.RXBPF_MIDCUR          = 0;
   cc2420_RXCTRL1_reg.RXBPF_LOCUR           = 1; // use this setting as per datasheet
   cc2420_RXCTRL1_reg.reserved_w0           = 0;
   cc2420_spiWriteReg(
      CC2420_RXCTRL1_ADDR,
      &radio_vars.radioStatusByte,
      *(uint16_t*)&cc2420_RXCTRL1_reg
   );
}

//===== RF admin

void radio_setFrequency(uint8_t frequency, radio_freq_t tx_or_rx) {
   cc2420_FSCTRL_reg_t cc2420_FSCTRL_reg;
   
   // change state
   radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
   
   cc2420_FSCTRL_reg.FREQ         = frequency-11;
   cc2420_FSCTRL_reg.FREQ        *= 5;
   cc2420_FSCTRL_reg.FREQ        += 357;
   cc2420_FSCTRL_reg.LOCK_STATUS  = 0;
   cc2420_FSCTRL_reg.LOCK_LENGTH  = 0;
   cc2420_FSCTRL_reg.CAL_RUNNING  = 0;
   cc2420_FSCTRL_reg.CAL_DONE     = 0;
   cc2420_FSCTRL_reg.LOCK_THR     = 1;
   
   cc2420_spiWriteReg(
      CC2420_FSCTRL_ADDR,
      &radio_vars.radioStatusByte,
      *(uint16_t*)&cc2420_FSCTRL_reg
   );
   
   // change state
   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {   
   cc2420_spiStrobe(CC2420_SXOSCON, &radio_vars.radioStatusByte);
   while (radio_vars.radioStatusByte.xosc16m_stable==0) {
      cc2420_spiStrobe(CC2420_SNOP, &radio_vars.radioStatusByte);
   }
}

void radio_rfOff(void) {
   
   // change state
   radio_vars.state = RADIOSTATE_TURNING_OFF;
   
   cc2420_spiStrobe(CC2420_SRFOFF, &radio_vars.radioStatusByte);
   // poipoipoi wait until off
   
   // wiggle debug pin
   debugpins_radio_clr();
   leds_radio_off();
   
   // change state
   radio_vars.state = RADIOSTATE_RFOFF;
}

int8_t radio_getFrequencyOffset(void){

    // not available
    return 0;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;
   
   cc2420_spiStrobe(CC2420_SFLUSHTX, &radio_vars.radioStatusByte);
   cc2420_spiWriteFifo(&radio_vars.radioStatusByte, packet, len, CC2420_TXFIFO_ADDR);
   
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   
   // I don't fully understand how the CC2420_STXCA the can be used here.
   
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   
   cc2420_spiStrobe(CC2420_STXON, &radio_vars.radioStatusByte);
}

//===== RX

void radio_rxEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_RX;

   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
}

void radio_rxNow(void) {
  // change state
  radio_vars.state = RADIOSTATE_LISTENING;

  // put radio in reception mode
  cc2420_spiStrobe(CC2420_SRXON, &radio_vars.radioStatusByte);
  cc2420_spiStrobe(CC2420_SFLUSHRX, &radio_vars.radioStatusByte);

  // busy wait until radio really listening
  while (radio_vars.radioStatusByte.rssi_valid==0) {
     cc2420_spiStrobe(CC2420_SNOP, &radio_vars.radioStatusByte);
  }
}

void radio_getReceivedFrame(
      uint8_t* bufRead,
      uint8_t* lenRead,
      uint8_t  maxBufLen,
      int8_t*  rssi,
      uint8_t* lqi,
      bool*    crc
   ) {
   
   // read the received packet from the RXFIFO
   cc2420_spiReadRxFifo(&radio_vars.radioStatusByte, bufRead, lenRead, maxBufLen);
   
   // On reception, when MODEMCTRL0.AUTOCRC is set, the CC2420 replaces the
   // received CRC by:
   // - [1B] the rssi, a signed value. The actual value in dBm is that - 45.
   // - [1B] whether CRC checked (bit 7) and LQI (bit 6-0)
   *rssi  =  *(bufRead+*lenRead-2);
   *rssi -= 45;
   *crc   = ((*(bufRead+*lenRead-1))&0x80)>>7;
   *lqi   =  (*(bufRead+*lenRead-1))&0x7f;
}

//=========================== private =========================================

//=========================== callbacks =======================================
