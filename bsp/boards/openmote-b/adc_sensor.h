/**
    \brief Declaration of the OpenMote-CC2538 ADC temperature sensor driver.
    \author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
*/

#ifndef __ADC_SENSOR_H__
#define __ADC_SENSOR_H__

#include "board_info.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

void adc_sensor_init(void);
uint16_t adc_sens_read_temperature(void);
float adc_sens_convert_temperature(uint16_t cputemp);

#endif // __ADC_SENSOR_H__
