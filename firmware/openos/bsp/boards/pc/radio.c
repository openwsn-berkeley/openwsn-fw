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
   printf("TODO radio_init\r\n");
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
   // poipoipoi stub
   printf("TODO radio_setOverflowCb\r\n");
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
   // poipoipoi stub
   printf("TODO radio_setCompareCb\r\n");
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
   // poipoipoi stub
   printf("TODO radio_setStartFrameCb\r\n");
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   // poipoipoi stub
   printf("TODO radio_setEndFrameCb\r\n");
}

//===== reset

void radio_reset() {
   // poipoipoi stub
   printf("TODO radio_reset\r\n");
}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
   // poipoipoi stub
   printf("TODO radio_startTimer\r\n");
}

PORT_TIMER_WIDTH radio_getTimerValue() {
   // poipoipoi stub
   printf("TODO radio_getTimerValue\r\n");
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {
   // poipoipoi stub
   printf("TODO radio_setTimerPeriod\r\n");
}

PORT_TIMER_WIDTH radio_getTimerPeriod() {
   // poipoipoi stub
   printf("TODO radio_getTimerPeriod\r\n");
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
   // poipoipoi stub
   printf("TODO radio_setFrequency\r\n");
}

void radio_rfOn() {
   // poipoipoi stub
   printf("TODO radio_rfOn\r\n");
}

void radio_rfOff() {
   // poipoipoi stub
   printf("TODO radio_rfOff\r\n");
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   // poipoipoi stub
   printf("TODO radio_loadPacket\r\n");
}

void radio_txEnable() {
   // poipoipoi stub
   printf("TODO radio_txEnable\r\n");
}

void radio_txNow() {
   // poipoipoi stub
   printf("TODO radio_txNow\r\n");
}

//===== RX

void radio_rxEnable() {
   // poipoipoi stub
   printf("TODO radio_rxEnable\r\n");
}

void radio_rxNow() {
   // poipoipoi stub
   printf("TODO radio_rxNow\r\n");
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                            uint8_t* pCrc) {
   // poipoipoi stub
   printf("TODO radio_getReceivedFrame\r\n");
}

//=========================== private =========================================
