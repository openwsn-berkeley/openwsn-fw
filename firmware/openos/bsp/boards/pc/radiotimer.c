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
   printf("TODO radiotimer_init\r\n");
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   // poipoipoi stub
   printf("TODO radiotimer_setOverflowCb\r\n");
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   // poipoipoi stub
   printf("TODO radiotimer_setCompareCb\r\n");
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   // poipoipoi stub
   printf("TODO radiotimer_setStartFrameCb\r\n");
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   // poipoipoi stub
   printf("TODO radiotimer_setEndFrameCb\r\n");
}

void radiotimer_start(uint16_t period) {
   // poipoipoi stub
   printf("TODO radiotimer_start\r\n");
}

//===== direct access

uint16_t radiotimer_getValue() {
   // poipoipoi stub
   printf("TODO radiotimer_getValue\r\n");
}

void radiotimer_setPeriod(uint16_t period) {
   // poipoipoi stub
   printf("TODO radiotimer_setPeriod\r\n");
}

uint16_t radiotimer_getPeriod() {
   // poipoipoi stub
   printf("TODO radiotimer_getPeriod\r\n");
}

//===== compare

void radiotimer_schedule(uint16_t offset) {
   // poipoipoi stub
   printf("TODO radiotimer_schedule\r\n");
}

void radiotimer_cancel() {
   // poipoipoi stub
   printf("TODO radiotimer_cancel\r\n");
}

//===== capture

uint16_t radiotimer_getCapturedTime() {
   // poipoipoi stub
   printf("TODO radiotimer_getCapturedTime\r\n");
}

//=========================== private =========================================