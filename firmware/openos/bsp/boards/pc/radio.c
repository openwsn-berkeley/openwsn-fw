/**
\brief PC-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "radio.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   radiotimer_capture_cbt    startFrame_cb;
   radiotimer_capture_cbt    endFrame_cb;
   radio_state_t             state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radio_init() {

   // clear variables
   memset(&radio_vars,0,sizeof(radio_vars_t));
   
   // poipoipoi stub
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
   // poipoipoi stub
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
   // poipoipoi stub
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
   // poipoipoi stub
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   // poipoipoi stub
}

//===== reset

void radio_reset() {
   // poipoipoi stub
}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
   // poipoipoi stub
}

PORT_TIMER_WIDTH radio_getTimerValue() {
   // poipoipoi stub
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {
   // poipoipoi stub
}

PORT_TIMER_WIDTH radio_getTimerPeriod() {
   // poipoipoi stub
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
   // poipoipoi stub
}

void radio_rfOn() {
   // poipoipoi stub
}

void radio_rfOff() {
   // poipoipoi stub
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   // poipoipoi stub
}

void radio_txEnable() {
   // poipoipoi stub
}

void radio_txNow() {
   // poipoipoi stub
}

//===== RX

void radio_rxEnable() {
   // poipoipoi stub
}

void radio_rxNow() {
   // poipoipoi stub
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                            uint8_t* pCrc) {
   // poipoipoi stub
}

//=========================== private =========================================
