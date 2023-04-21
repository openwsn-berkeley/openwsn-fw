/**
    \brief Declaration of the nrf52480 ADC driver.
    \author Frank Senf <frank.senf@imms.de>, July 2018.
*/

#ifndef __ADC_SENSOR_H__
#define __ADC_SENSOR_H__

#include "board_info.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

bool adc_sens_init(void);
uint16_t adc_sens_read_battery(void);
float adc_sens_convert_battery(uint16_t raw);

uint16_t adc_sens_read_temperature(void);
float adc_sens_convert_temperature(uint16_t cpu_temp_raw);


#endif // __ADC_SENSOR_H__
