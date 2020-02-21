#ifndef __OPTICAL_H
#define __OPTICAL_H

#include <stdint.h>

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//==== admin
void optical_init(void);
uint8_t optical_getCalibrationFinshed(void);
void optical_enable(void);

#endif
