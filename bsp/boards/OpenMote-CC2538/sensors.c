#include "opendefs.h"
#include "adxl346.h"
#include "max44009.h"
#include "sht21.h"
#include "adc_sensor.h"
#include "opensensors.h"

const uint8_t temperature[] = "t";
const uint8_t humidity[] = "h";
const uint8_t light[] = "l";
const uint8_t x[] = "x";
const uint8_t y[] = "y";
const uint8_t z[] = "z";
const uint8_t cputemp[] = "c";

void sensors_init(void) {

   opensensors_init();
   // if (sht21_is_present()==1) {
      // sht21_init();
      // opensensors_register(
         // sizeof(temperature)-1,
         // (uint8_t*)(&temperature),
         // &sht21_read_temperature,
         // &sht21_convert_temperature
      // );
      // opensensors_register(
         // sizeof(humidity)-1,
         // (uint8_t*)(&humidity),
         // &sht21_read_humidity,
         // &sht21_convert_humidity
      // );
   // }
   // if (max44009_is_present()==1) {
      // max44009_init();
      // opensensors_register(
         // sizeof(light)-1,
         // (uint8_t*)(&light),
         // &max44009_read_light,
         // &max44009_convert_light
      // );
   // }
   // if (adxl346_is_present()==1) {
      // adxl346_init();
      // opensensors_register(
         // sizeof(x)-1,
         // (uint8_t*)(&x),
         // &adxl346_read_x,
         // NULL
      // );
      // opensensors_register(
         // sizeof(y)-1,
         // (uint8_t*)(&y),
         // &adxl346_read_y,
         // NULL
      // );
      // opensensors_register(
         // sizeof(z)-1,
         // (uint8_t*)(&z),
         // &adxl346_read_z,
         // NULL
      // );
   // }

   adc_sensor_init();
   opensensors_register(
      sizeof(cputemp)-1,
      (uint8_t*)(&cputemp),
      &adc_sens_read_temperature,
      &adc_sens_convert_temperature
   );

}
