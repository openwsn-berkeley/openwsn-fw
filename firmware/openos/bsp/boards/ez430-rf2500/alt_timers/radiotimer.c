/**
\brief eZ430-RF2500-specific definition of the "radiotimer" bsp module.

\author Chuang Qian <cqian@berkeley.edu>, April 2012.

*/

//#include "io430.h"
#include "stdio.h"
#include "string.h"
#include "radiotimer.h"
#include "lptmr.h"

//=========================== variables =======================================

/*
typedef struct {
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;
*/

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   lptimer_init();
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   lptimer_radiotimer_setOverflowCb(cb);
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   lptimer_radiotimer_setCompareCb(cb);
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   lptimer_radiotimer_setStartFrameCb(cb);
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   lptimer_radiotimer_setEndFrameCb(cb);
}

void radiotimer_start(uint16_t period) {
   lptimer_radiotimer_start(period);
}

//===== direct access

uint16_t radiotimer_getValue() {
   return lptimer_radiotimer_getValue();
}

void radiotimer_setPeriod(uint16_t period) {
   lptimer_radiotimer_setPeriod(period);
}

uint16_t radiotimer_getPeriod() {
   return lptimer_radiotimer_getPeriod();
}

//===== compare

void radiotimer_schedule(uint16_t offset) {
   lptimer_radiotimer_schedule(offset);
}

void radiotimer_cancel() {
   lptimer_radiotimer_cancel();
}

//===== capture

inline uint16_t radiotimer_getCapturedTime() {
   return lptimer_radiotimer_getCapturedTime();
}

//=========================== private =========================================