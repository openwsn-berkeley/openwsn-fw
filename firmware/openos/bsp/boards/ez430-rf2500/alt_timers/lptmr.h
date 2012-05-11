/**
\brief A BSP timer module which abstracts away the "bsp_timer" and "radiotimer"
       modules behind a single timer.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
*/

#ifndef __LPTMR_H
#define __LPTMR_H

#include "bsp_timer.h"
#include "radiotimer.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*lptmr_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void               lptimer_init();
// from bsp_timer
void               lptimer_bsp_timer_set_callback(bsp_timer_cbt cb);
void               lptimer_bsp_timer_reset();
void               lptimer_bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks);
void               lptimer_bsp_timer_cancel_schedule();
PORT_TIMER_WIDTH   lptimer_bsp_timer_get_currentValue();
// from radiotimer
void               lptimer_radiotimer_setOverflowCb(radiotimer_compare_cbt cb);
void               lptimer_radiotimer_setCompareCb(radiotimer_compare_cbt cb);
void               lptimer_radiotimer_setStartFrameCb(radiotimer_capture_cbt cb);
void               lptimer_radiotimer_setEndFrameCb(radiotimer_capture_cbt cb);
void               lptimer_radiotimer_start(uint16_t period);
uint16_t           lptimer_radiotimer_getValue();
void               lptimer_radiotimer_setPeriod(uint16_t period);
uint16_t           lptimer_radiotimer_getPeriod();
void               lptimer_radiotimer_schedule(uint16_t offset);
void               lptimer_radiotimer_cancel();
uint16_t           lptimer_radiotimer_getCapturedTime();

#endif


