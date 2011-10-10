#ifndef __SENSITIVE_ACCEL_TEMPERATURE_H
#define __SENSITIVE_ACCEL_TEMPERATURE_H

/**
\addtogroup drivers
\{
\addtogroup SensitiveAccel
\{
*/

#include "stdint.h"

//=========================== define ==========================================

#define SENSITIVE_ACCEL_TEMPERATURE_DATALEN 10

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void sensitive_accel_temperature_init();
void sensitive_accel_temperature_disable();
void sensitive_accel_temperature_get_config();
void sensitive_accel_temperature_get_measurement(uint8_t* spaceToWrite);

/**
\}
\}
*/

#endif
