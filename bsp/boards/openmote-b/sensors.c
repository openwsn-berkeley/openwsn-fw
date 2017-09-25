/**
    \brief Definition of the "sensors" board-specific driver.
    \author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
    \author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2017.
*/

#include "adc_sensor.h"
#include "board.h"
#include "sensors.h"

#include "si70x.h"

//=========================== defines =========================================

//=========================== typedef =========================================

//=========================== variables =======================================

sensors_vars_t sensors_vars;

//=========================== prototype =======================================

//=========================== public ==========================================

/**
   \brief Initialize sensors on the board
*/
void sensors_init(void) {
   
   memset(&sensors_vars,0,sizeof(sensors_vars_t));
   
   if (si70x_is_present()==1) {
      si70x_init();
      sensors_vars.sensorsTypes[SENSOR_TEMPERATURE] = 1;
      sensors_vars.sensorsTypes[SENSOR_HUMIDITY] = 1;
   }
    
   adc_sensor_init();
   sensors_vars.sensorsTypes[SENSOR_ADCTEMPERATURE] = 1;
   
}

/**
   \brief Returns a bool value indicating if a given sensor is present
   \param[in] sensorType sensor type polled.
   \param[out] returnVal presence of the sensor.
*/
bool sensors_is_present(uint8_t sensorType) {
   return sensors_vars.sensorsTypes[sensorType];
}

/**
   \brief Returns the callback for reading data from a given sensor
   \param[in] sensorType sensor type used to associate the callback.
   \param[out] callback for reading data.
*/
callbackRead_cbt sensors_getCallbackRead(uint8_t sensorType) {
   
   switch (sensorType) {
      case SENSOR_TEMPERATURE:
         return &si70x_read_temperature;
      case SENSOR_HUMIDITY:
         return &si70x_read_humidity;
      case SENSOR_ADCTEMPERATURE:
         return &adc_sens_read_temperature;
      default:
         return NULL;
   }
   
}

/**
   \brief Returns the callback for converting data from a given sensor
   \param[in] sensorType sensor type used to associate the callback.
   \param[out] callback for converting data.
*/
callbackConvert_cbt sensors_getCallbackConvert(uint8_t sensorType) {
   
   switch (sensorType) {
      case SENSOR_TEMPERATURE:
         return &si70x_convert_temperature;
      case SENSOR_HUMIDITY:
         return &si70x_convert_humidity;
      case SENSOR_ADCTEMPERATURE:
         return &adc_sens_convert_temperature;
      default:
         return NULL;
   }
   
}

//=========================== private =========================================
