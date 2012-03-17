/**
\brief xpressohack-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdio.h"
#include "string.h"
#include "radiotimer.h"

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflowCb;
   radiotimer_compare_cbt    compareCb;
   radiotimer_capture_cbt    startFrameCb;
   radiotimer_capture_cbt    endFrameCb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   // poipoi
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   // poipoi
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   // poipoi
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   // poipoi
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   // poipoi
}

void radiotimer_start(uint16_t period) {
   // poipoi
}

//===== direct access

uint16_t radiotimer_getValue() {
   // poipoi
   return 0;
}

void radiotimer_setPeriod(uint16_t period) {
   // poipoi
}

uint16_t radiotimer_getPeriod() {
   // poipoi
   return 0;
}

//===== compare

void radiotimer_schedule(uint16_t offset) {
   // poipoi
}

void radiotimer_cancel() {
   // poipoi
}

//===== capture

inline uint16_t radiotimer_getCapturedTime() {
   // poipoi
   return 0;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
