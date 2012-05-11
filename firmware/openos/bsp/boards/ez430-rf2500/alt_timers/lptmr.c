/**
\brief A BSP timer module which abstracts away the "bsp_timer" and "radiotimer"
       modules behind a single timer.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
*/

#include "lptmr.h"
#include "openwsn.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   bool initialized;
} lptmr_vars_t;

lptmr_vars_t lptmr_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin
void lptimer_init() {
   
   // only initialize once
   if (lptmr_vars.initialized==FALSE) {
      // clear module variables
      memset(&lptmr_vars,0,sizeof(lptmr_vars_t));
      
      // TODO
   }
}

//===== from bsp_timer
void lptimer_bsp_timer_set_callback(bsp_timer_cbt cb) {
   // TODO
}

void lptimer_bsp_timer_reset() {
   // TODO
}

void lptimer_bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks) {
   // TODO
}

void lptimer_bsp_timer_cancel_schedule() {
   // TODO
}

PORT_TIMER_WIDTH lptimer_bsp_timer_get_currentValue() {
   // TODO
   return 0;
}

//===== from radiotimer
void lptimer_radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   // TODO
}

void lptimer_radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   // TODO
}

void lptimer_radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   // TODO
}

void lptimer_radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   // TODO
}

void lptimer_radiotimer_start(uint16_t period) {
   // TODO
}

uint16_t lptimer_radiotimer_getValue() {
   return 0;
   // TODO
}

void lptimer_radiotimer_setPeriod(uint16_t period) {
   // TODO
}

uint16_t lptimer_radiotimer_getPeriod() {
   // TODO
   return 0;
}

void lptimer_radiotimer_schedule(uint16_t offset) {
   // TODO
}

void lptimer_radiotimer_cancel() {
   // TODO
}

uint16_t lptimer_radiotimer_getCapturedTime() {
   return 0;
   // TODO
}

//=========================== private =========================================

//=========================== interrupts ======================================

uint8_t bsp_timer_isr() {
   while(1);
   // TODO
}

uint8_t radiotimer_isr() {
   while(1);
   // TODO
}