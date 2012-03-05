/**
\brief Cross-platform declaration "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __RADIOTIMER_H
#define __RADIOTIMER_H

#include "stdint.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*radiotimer_compare_cbt)();
typedef void (*radiotimer_capture_cbt)(uint16_t timestamp);

//=========================== variables =======================================

//=========================== prototypes ======================================

void     radiotimer_init();
void     radiotimer_setOverflowCb(radiotimer_compare_cbt cb);
void     radiotimer_setCompareCb(radiotimer_compare_cbt cb);
void     radiotimer_setStartFrameCb(radiotimer_capture_cbt cb);
void     radiotimer_setEndFrameCb(radiotimer_capture_cbt cb);
void     radiotimer_start(uint16_t period);
uint16_t radiotimer_getValue();
void     radiotimer_setPeriod(uint16_t period);
uint16_t radiotimer_getPeriod();
void     radiotimer_schedule(uint16_t offset);
void     radiotimer_cancel();
uint16_t radiotimer_getCapturedTime();

uint8_t  radiotimer_isr();

#endif
