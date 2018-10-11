/**
   \brief Definition of the OpenMote-CC2538 ADC temperature sensor driver.
   \author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
*/

#include <headers/hw_cctest.h>
#include <headers/hw_rfcore_xreg.h>

#include <source/adc.h>

#include "adc_sensor.h"

//=========================== defines =========================================

#define CONST 0.58134 //(VREF / 2047) = (1190 / 2047), VREF from Datasheet
#define OFFSET_DATASHEET_25C 827 // 1422*CONST, from Datasheet
#define TEMP_COEFF (CONST * 4.2) // From Datasheet
#define OFFSET_0C (OFFSET_DATASHEET_25C - (25 * TEMP_COEFF))

//=========================== variables =======================================

//=========================== prototype =======================================

//=========================== public ==========================================

/**
   \brief Initialize the sensor
*/
void adc_sensor_init(void) {
   HWREG(CCTEST_TR0) |= CCTEST_TR0_ADCTM;
   HWREG(RFCORE_XREG_ATEST) = 0x01;
   SOCADCSingleConfigure(SOCADC_12_BIT, SOCADC_REF_INTERNAL);
   adc_sens_read_temperature();
}

/**
   \brief Read rough data from sensor
   \param[out] ui16Dummy rough data.
*/
uint16_t adc_sens_read_temperature(void) {
   uint16_t ui16Dummy;

   SOCADCSingleStart(SOCADC_TEMP_SENS);
   while(!SOCADCEndOfCOnversionGet());
   ui16Dummy = SOCADCDataGet() >> SOCADC_12_BIT_RSHIFT;
   return ui16Dummy;
}

/**
   \brief Convert rough data to human understandable
   \param[in] cputemp rough data.
   \param[out] the number of registered OpenSensors.
*/
float adc_sens_convert_temperature(uint16_t cputemp) {
   float dOutputVoltage;

   dOutputVoltage = cputemp * CONST;
   dOutputVoltage = ((dOutputVoltage - OFFSET_0C) / TEMP_COEFF);
   return dOutputVoltage;
}

//=========================== private =========================================
