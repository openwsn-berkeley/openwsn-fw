/**
\brief PC-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "radiotimer.h"

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
   
   // poipoipoi stub
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   // poipoipoi stub
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   // poipoipoi stub
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   // poipoipoi stub
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   // poipoipoi stub
}

void radiotimer_start(uint16_t period) {
   // poipoipoi stub
}

//===== direct access

uint16_t radiotimer_getValue() {
   // poipoipoi stub
}

void radiotimer_setPeriod(uint16_t period) {
   // poipoipoi stub
}

uint16_t radiotimer_getPeriod() {
   // poipoipoi stub
}

//===== compare

void radiotimer_schedule(uint16_t offset) {
   // poipoipoi stub
}

void radiotimer_cancel() {
   // poipoipoi stub
}

//===== capture

uint16_t radiotimer_getCapturedTime() {
   // poipoipoi stub
}

//=========================== private =========================================