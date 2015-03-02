/**
    \brief Definition of the "sensors" board-specif driver.
    \author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
*/

#include "opendefs.h"
#include "adxl346.h"
#include "max44009.h"
#include "sht21.h"
#include "adc_sensor.h"
#include "opensensors.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototype =======================================

//=========================== public ==========================================

/**
   \brief Initialize sensors on the board, registering them through opensensors
*/
void sensors_init(void) {

   opensensors_init();
   if (sht21_is_present()==1) {
      sht21_init();
      opensensors_register(
         SENSOR_TEMPERATURE,
         &sht21_read_temperature,
         &sht21_convert_temperature
      );
      opensensors_register(
         SENSOR_HUMIDITY,
         &sht21_read_humidity,
         &sht21_convert_humidity
      );
   }
   if (max44009_is_present()==1) {
      max44009_init();
      opensensors_register(
         SENSOR_LIGHT,
         &max44009_read_light,
         &max44009_convert_light
      );
   }
   if (adxl346_is_present()==1) {
      adxl346_init();
      opensensors_register(
         SENSOR_XACCELERATION,
         &adxl346_read_x,
         NULL
      );
      opensensors_register(
         SENSOR_YACCELERATION,
         &adxl346_read_y,
         NULL
      );
      opensensors_register(
         SENSOR_ZACCELERATION,
         &adxl346_read_z,
         NULL
      );
   }

   adc_sensor_init();
   opensensors_register(
      SENSOR_ADCTEMPERATURE,
      &adc_sens_read_temperature,
      &adc_sens_convert_temperature
   );

}

//=========================== private =========================================