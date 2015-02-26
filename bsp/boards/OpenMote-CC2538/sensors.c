#include "opendefs.h"
#include "adxl346.h"
#include "max44009.h"
#include "sht21.h"
#include "adc_sensor.h"

const uint8_t temperature[] = "t";
const uint8_t humidity[] = "h";
const uint8_t light[] = "l";
const uint8_t x[] = "x";
const uint8_t y[] = "y";
const uint8_t z[] = "z";
const uint8_t cputemp[] = "c";

void sensors_init(void) {

   if (sht21_is_present()==1) {
      sht21_init();
   }
   if (max44009_is_present()==1) {
      max44009_init();
   }
   if (adxl346_is_present()==1) {
      adxl346_init();
   }

   adc_sensor_init();

}
