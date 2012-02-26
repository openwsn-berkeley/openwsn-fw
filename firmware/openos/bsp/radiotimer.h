/**
\brief Cross-platform declaration "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __RADIOTIMER_H
#define __RADIOTIMER_H

#include "stdint.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void     radiotimer_init(uint16_t period);
void     radiotimer_schedule(uint16_t offset);
void     radiotimer_cancel();
uint16_t radiotimer_getCapturedTime();

#endif
