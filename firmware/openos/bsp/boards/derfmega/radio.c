/**
\brief derfmega definition of the "radio" bsp module. Adapted from the AT86RF231 code since its the same register layout and all

\author Kevin Weekly June 2012.
*/

#include <avr/io.h>
#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   radiotimer_capture_cbt    startFrame_cb;
   radiotimer_capture_cbt    endFrame_cb;
   radio_state_t             state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

#define radio_internalWriteReg(R,V) R = V
#define radio_internalReadReg(R) (R)
void    radio_internalWriteTxFifo(uint8_t* bufToWrite, uint8_t lenToWrite);
void    radio_internalReadRxFifo(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                            uint8_t* pLqi);

uint8_t radio_rx_start_isr();
uint8_t radio_trx_end_isr();
//=========================== public ==========================================

//===== admin

void radio_init() {
	// turn on radio power and reset
   PRR1 &= ~(1<<PRTRX24);
   TRXPR |= 0x01;
   while (TRXPR & 0x01);

   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // change state
   radio_vars.state          = RADIOSTATE_STOPPED;

   // configure the radio
   radio_internalWriteReg(TRX_STATE, CMD_FORCE_TRX_OFF);    // turn radio off

   
   radio_internalWriteReg(IRQ_MASK, 0b01001100); // enable TX_END,RX_START, RX_END interrupts
   radio_internalWriteReg(IRQ_STATUS,0xFF);                       // clear all interrupts
   radio_internalWriteReg(ANT_DIV, RADIO_CHIP_ANTENNA);     // use chip antenna

   radio_internalWriteReg(TRX_CTRL_1, 0x20);                // have the radio calculate CRC
   //busy wait until radio status is TRX_OFF
   while((radio_internalReadReg(TRX_STATUS) & 0x1F) != TRX_OFF);
   
   // change state
   radio_vars.state          = RADIOSTATE_RFOFF;
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset() {
   TRXPR |= 0x01; // force tranceiver reset
}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
   radiotimer_start(period);
}

PORT_TIMER_WIDTH radio_getTimerValue() {
   return radiotimer_getValue();
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {

   radiotimer_setPeriod(period);
}

PORT_TIMER_WIDTH radio_getTimerPeriod() {
   return radiotimer_getPeriod();
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
   // change state
   radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
   
   // configure the radio to the right frequecy
   radio_internalWriteReg(PHY_CC_CCA,0x20+frequency);
   
   // change state
   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn() {
   PRR1 &= ~_BV(PRTRX24);
}

void radio_rfOff() {
   // change state
   radio_vars.state = RADIOSTATE_TURNING_OFF;
   // turn radio off
   radio_internalWriteReg(TRX_STATE, CMD_FORCE_TRX_OFF);
   //radio_spiWriteReg(RG_TRX_STATE, CMD_TRX_OFF);
   while((radio_internalReadReg(TRX_STATUS) & 0x1F) != TRX_OFF); // busy wait until done
   
   // wiggle debug pin
   //debugpins_radio_clr();
   leds_radio_off();
   
   // change state
   radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;
   
   // load packet in TXFIFO
   radio_internalWriteTxFifo(packet,len);
   
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   //debugpins_radio_set();
   leds_radio_on();
   
   // turn on radio's PLL
   radio_internalWriteReg(TRX_STATE, CMD_PLL_ON);
   while((radio_internalReadReg(TRX_STATUS) & 0x1F) != PLL_ON); // busy wait until done
   
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow() {
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   
   // send packet by forcing state to TX_START
   radio_internalWriteReg(TRX_STATE, CMD_TX_START);
   // The AT86RF231 does not generate an interrupt when the radio transmits the
   // SFD, which messes up the MAC state machine. The danger is that, if we leave
   // this funtion like this, any radio watchdog timer will expire.
   // Instead, we cheat an mimick a start of frame event by calling
   // ieee154e_startOfFrame from here. This also means that software can never catch
   // a radio glitch by which #radio_txEnable would not be followed by a packet being
   // transmitted (I've never seen that).
   if (radio_vars.startFrame_cb!=NULL) {
      // call the callback
      radio_vars.startFrame_cb(radiotimer_getCapturedTime());
   }
}

//===== RX

void radio_rxEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_RX;
   
   // put radio in reception mode
   radio_internalWriteReg(TRX_STATE, CMD_RX_ON);
   // wiggle debug pin
   //debugpins_radio_set();
   leds_radio_on();
   
   // busy wait until radio really listening
   while((radio_internalReadReg(TRX_STATUS) & 0x1F) != RX_ON);
   
   // change state
   radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow() {
   // nothing to do
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {
   uint8_t temp_reg_value;
   
   //===== crc
   temp_reg_value  = radio_internalReadReg(PHY_RSSI);
   *pCrc           = (temp_reg_value & 0x80)>>7;  // msb is whether packet passed CRC
   
   //===== rssi
   // as per section 8.4.3 of the AT86RF231, the RSSI is calculate as:
   // -91 + ED [dBm]
   temp_reg_value  = radio_internalReadReg(PHY_ED_LEVEL);
   *pRssi          = -91 + temp_reg_value;
   
   //===== packet
   radio_internalReadRxFifo(pBufRead,
                       pLenRead,
                       maxBufLen,
                       pLqi);
}

//=========================== private =========================================

/** for testing purposes, remove if not needed anymore**/

void radio_internalWriteTxFifo(uint8_t* bufToWrite, uint8_t  lenToWrite) {
	TRXFBST = lenToWrite;
	memcpy(&TRXFBST + 1,bufToWrite,lenToWrite);
}



void radio_internalReadRxFifo(uint8_t* pBufRead,
                         uint8_t* pLenRead,
                         uint8_t  maxBufLen,
                         uint8_t* pLqi) {
							 
	*pLenRead = TST_RX_LENGTH;
	memcpy(pBufRead,&TRXFBST,*pLenRead); 
	//poipoi, see if LQI is included in the length
	*pLqi = *(TRXFBST + pLenRead);
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

uint8_t radio_isr() {
	// should not be called
	return 0;
}

uint8_t radio_rx_start_isr() {
   PORT_TIMER_WIDTH capturedTime;
   // capture the time
   capturedTime = radiotimer_getCapturedTime();	
	radio_vars.state = RADIOSTATE_RECEIVING;
	if (radio_vars.startFrame_cb!=NULL) {
		// call the callback
		radio_vars.startFrame_cb(capturedTime);
		// kick the OS
		return 1;
	}
	return 0;
}

uint8_t radio_trx_end_isr() {
   PORT_TIMER_WIDTH capturedTime;
   // capture the time
   capturedTime = radiotimer_getCapturedTime();	
    radio_vars.state = RADIOSTATE_TXRX_DONE;
    if (radio_vars.endFrame_cb!=NULL) {
	    // call the callback
	    radio_vars.endFrame_cb(capturedTime);
	    // kick the OS
	    return 1;
    }
	return 0;
}