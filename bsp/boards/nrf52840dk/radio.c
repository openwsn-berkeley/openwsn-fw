/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "radio" bsp module.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "sdk/components/boards/pca10056.h"
#include "sdk/components/boards/boards.h"
#include "sdk/components/proprietary_rf/esb/nrf_esb.h"
#include "sdk/components/drivers_nrf/radio_config/radio_config.h"
#include "sdk/components/proprietary_rf/esb/nrf_esb_error_codes.h"
#include "sdk/modules/nrfx/mdk/nrf52840.h"

#include "app_config.h"
#include "leds.h"
#include "radio.h"
#include "board.h"
#include "board_info.h"
#include "debugpins.h"
#include "sctimer.h"

//=========================== defines =========================================

/* Bit Masks for the last byte in the RX FIFO */
//#define CRC_BIT_MASK 0x80
//#define LQI_BIT_MASK 0x7F

/* RSSI Offset */
//#define RSSI_OFFSET 73
//#define CHECKSUM_LEN 2

//=========================== variables =======================================

typedef struct {
   radio_capture_cbt         startFrame_cb;
   radio_capture_cbt         endFrame_cb;
   radio_state_t             state; 
} radio_vars_t;

radio_vars_t radio_vars;

nrf_esb_payload_t tx_payload;
nrf_esb_payload_t rx_payload;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radio_init(void) {
   
   nrf_esb_config_t esb_config = NRF_ESB_DEFAULT_CONFIG;

   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   radio_configure();
   nrf_esb_init(&esb_config);

}

void radio_setStartFrameCb(radio_capture_cbt cb) {
   radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radio_capture_cbt cb) {
   radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset(void) {

}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {

   radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;

   NRF_RADIO->FREQUENCY = frequency;

   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {
   //radio_on();
}

void radio_rfOff(void) {
   
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {
   
  radio_vars.state = RADIOSTATE_LOADING_PACKET;

  if (len > 252)
  {
    leds_sync_toggle();
  } 
  else 
  {

    nrf_esb_stop_rx();
    nrf_esb_disable();
    
    tx_payload.noack = true;
    tx_payload.pipe = 0;
    tx_payload.length = len;
    memcpy(tx_payload.data, packet, len);

    if(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS){
      leds_radio_toggle();
      radio_vars.state = RADIOSTATE_PACKET_LOADED;
    }
  }
}

void radio_txEnable(void) {

   radio_vars.state = RADIOSTATE_ENABLING_TX;

   NRF_RADIO->TASKS_TXEN = 1;

   radio_vars.state = RADIOSTATE_TX_ENABLED;
}


void radio_txNow(void) {

   radio_vars.state = RADIOSTATE_TRANSMITTING;

   if (nrf_esb_start_tx() != NRF_SUCCESS)
      leds_debug_toggle();   
}

//===== RX

void radio_rxEnable(void) {
   radio_vars.state = RADIOSTATE_ENABLING_RX;

   radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
  
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {
   
   radio_vars.state = RADIOSTATE_RECEIVING;


   radio_vars.state = RADIOSTATE_TXRX_DONE; 
   
}

//=========================== private =========================================

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr(void) {
   return DO_NOT_KICK_SCHEDULER;
}
