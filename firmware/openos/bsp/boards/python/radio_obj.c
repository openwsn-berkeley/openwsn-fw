/**
\brief PC-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "radio_obj.h"

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

void radio_setOverflowCb(OpenMote* self, radiotimer_compare_cbt cb) {
 radiotimer_setOverflowCb(self, cb);
}

void radio_setCompareCb(OpenMote* self, radiotimer_compare_cbt cb) {
 radiotimer_setCompareCb(self, cb);
}

void radio_setStartFrameCb(OpenMote* self, radiotimer_capture_cbt cb) {
   radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(OpenMote* self, radiotimer_capture_cbt cb) {
   radio_vars.endFrame_cb    = cb;
}

//=========================== public ==========================================

//===== admin

void radio_init(OpenMote* self) {

   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_init,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

//===== reset

void radio_reset(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_reset,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

//===== timer

void radio_startTimer(OpenMote* self, PORT_TIMER_WIDTH period) {
   //opensim_requ_radio_startTimer_t requparams;
   
   // prepare request
   //requparams.period = period;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_startTimer,
                                    &requparams,
                                    sizeof(opensim_requ_radio_startTimer_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

PORT_TIMER_WIDTH radio_getTimerValue(OpenMote* self) {
   //opensim_repl_radio_getTimerValue_t replparams;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_getTimerValue,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radio_getTimerValue_t));
   */
   // TODO: replace by call to Python
   
   //return replparams.value;
   return 0;//poipoi
}

void radio_setTimerPeriod(OpenMote* self, PORT_TIMER_WIDTH period) {
   //opensim_requ_radio_setTimerPeriod_t requparams;
   
   // prepare request
   //requparams.period = period;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_setTimerPeriod,
                                    &requparams,
                                    sizeof(opensim_requ_radio_setTimerPeriod_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

PORT_TIMER_WIDTH radio_getTimerPeriod(OpenMote* self) {
   //opensim_repl_radio_getTimerPeriod_t replparams;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_getTimerPeriod,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radio_getTimerPeriod_t));
                                    
   */
   // TODO: replace by call to Python
   
   //return replparams.value;
   return 0;//poipoi
}

//===== RF admin

void radio_setFrequency(OpenMote* self, uint8_t frequency) {
   //opensim_requ_radio_setFrequency_t requparams;
   
   // prepare request
   //requparams.frequency = frequency;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_setFrequency,
                                    &requparams,
                                    sizeof(opensim_requ_radio_setFrequency_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void radio_rfOn(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_rfOn,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void radio_rfOff(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_rfOff,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

//===== TX

void radio_loadPacket(OpenMote* self, uint8_t* packet, uint8_t len) {
   //opensim_requ_radio_loadPacket_t requparams;
   
   //requparams.len = len;
   //memcpy(requparams.txBuffer,packet,len);
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_loadPacket,
                                    &requparams,
                                    sizeof(opensim_requ_radio_loadPacket_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void radio_txEnable(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_txEnable,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void radio_txNow(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_txNow,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

//===== RX

void radio_rxEnable(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_rxEnable,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void radio_rxNow(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_rxNow,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void radio_getReceivedFrame(OpenMote* self, uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                            uint8_t* pCrc) {
   /*
   uint8_t numBytesToWrite;
   
   //opensim_repl_radio_getReceivedFrame_t replparams;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_getReceivedFrame,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radio_getReceivedFrame_t));
   
   // TODO: replace by call to Python
   
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
   */
}

//=========================== interrupts ======================================

void radio_intr_startOfFrame(OpenMote* self, uint16_t capturedTime) {
   radio_vars.startFrame_cb(self, capturedTime);
}

void radio_intr_endOfFrame(OpenMote* self, uint16_t capturedTime) {
   radio_vars.endFrame_cb(self, capturedTime);
}

//=========================== private =========================================
