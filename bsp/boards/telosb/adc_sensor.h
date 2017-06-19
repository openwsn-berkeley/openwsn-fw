/**
\brief Definitions for the analog light sensors (total solar and photosynthetic)

\author Pedro Henrique Gomes <pedrohenriquegomes@gmail.com>
*/

#ifndef __ADC_SENSOR_H__
#define __ADC_SENSOR_H__

#include "board_info.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

void adc_sensor_init(void);
uint16_t adc_sens_read_photosynthetic(void);
float adc_sens_convert_photosynthetic(uint16_t light);
uint16_t adc_sens_read_total_solar(void);
float adc_sens_convert_total_solar(uint16_t light);

#endif // __ADC_SENSOR_H__
