/**
\brief PC-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "radio.h"
#include "opensim_proto.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   radiotimer_capture_cbt    startFrame_cb;
   radiotimer_capture_cbt    endFrame_cb;
   radio_state_t             state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

//=========================== callbacks =======================================

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

//=========================== public ==========================================

//===== admin

void radio_init() {

   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_init,
                                    0,
                                    0,
                                    0,
                                    0);
}

//===== reset

void radio_reset() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_reset,
                                    0,
                                    0,
                                    0,
                                    0);
}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
   opensim_requ_radio_startTimer_t requparams;
   
   // prepare request
   requparams.period = period;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_startTimer,
                                    &requparams,
                                    sizeof(opensim_requ_radio_startTimer_t),
                                    0,
                                    0);
}

PORT_TIMER_WIDTH radio_getTimerValue() {
   opensim_repl_radio_getTimerValue_t replparams;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_getTimerValue,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radio_getTimerValue_t));
                                    
   return replparams.value;
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {
   opensim_requ_radio_setTimerPeriod_t requparams;
   
   // prepare request
   requparams.period = period;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_setTimerPeriod,
                                    &requparams,
                                    sizeof(opensim_requ_radio_setTimerPeriod_t),
                                    0,
                                    0);
}

PORT_TIMER_WIDTH radio_getTimerPeriod() {
   opensim_repl_radio_getTimerPeriod_t replparams;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_getTimerPeriod,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radio_getTimerPeriod_t));
                                    
   return replparams.value;
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
   opensim_requ_radio_setFrequency_t requparams;
   
   // prepare request
   requparams.frequency = frequency;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_setFrequency,
                                    &requparams,
                                    sizeof(opensim_requ_radio_setFrequency_t),
                                    0,
                                    0);
}

void radio_rfOn() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_rfOn,
                                    0,
                                    0,
                                    0,
                                    0);
}

void radio_rfOff() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_rfOff,
                                    0,
                                    0,
                                    0,
                                    0);
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   opensim_requ_radio_loadPacket_t requparams;
   
   requparams.len = len;
   memcpy(requparams.txBuffer,packet,len);
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_loadPacket,
                                    &requparams,
                                    sizeof(opensim_requ_radio_loadPacket_t),
                                    0,
                                    0);
}

void radio_txEnable() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_txEnable,
                                    0,
                                    0,
                                    0,
                                    0);
}

void radio_txNow() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_txNow,
                                    0,
                                    0,
                                    0,
                                    0);
}

//===== RX

void radio_rxEnable() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_rxEnable,
                                    0,
                                    0,
                                    0,
                                    0);
}

void radio_rxNow() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_rxNow,
                                    0,
                                    0,
                                    0,
                                    0);
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                            uint8_t* pCrc) {
   uint8_t numBytesToWrite;
   
   opensim_repl_radio_getReceivedFrame_t replparams;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_getReceivedFrame,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radio_getReceivedFrame_t));
   
   if (maxBufLen>replparams.len) {
      numBytesToWrite=replparams.len;
   } else {
      numBytesToWrite=maxBufLen;
   }
   
   // write return values
   memcpy(pBufRead,replparams.rxBuffer,numBytesToWrite);
   *pLenRead = replparams.len;
   *pRssi    = replparams.rssi;
   *pLqi     = replparams.lqi;
   *pCrc     = replparams.crc;
}

//=========================== interrupts ======================================

void radio_intr_startOfFrame(uint16_t capturedTime) {
   radio_vars.startFrame_cb(capturedTime);
}

void radio_intr_endOfFrame(uint16_t capturedTime) {
   radio_vars.endFrame_cb(capturedTime);
}

//=========================== private =========================================
