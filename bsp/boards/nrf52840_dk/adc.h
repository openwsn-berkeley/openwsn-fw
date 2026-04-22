/**
    \brief Declaration of the nrf52480 ADC driver.
    \author Frank Senf <frank.senf@imms.de>, July 2018.
*/

#ifndef __ADC_H__
#define __ADC_H__

#include "board_info.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

void adc_init(void);
uint16_t adc_read(void);

#endif // __ADC_SENSOR_H__
