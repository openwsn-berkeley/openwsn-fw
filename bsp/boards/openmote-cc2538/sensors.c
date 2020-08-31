/**
    \brief Definition of the "sensors" board-specific driver.
    \author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
*/

#include "config.h"

#if BOARD_SENSORS_ENABLED

#include "adc_sensor.h"
#include "board.h"
#include "sensors.h"

#include "adxl346.h"
#include "max44009.h"
#include "sht21.h"

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

    memset(&sensors_vars, 0, sizeof(sensors_vars_t));

    if (sht21_is_present() == 1) {
        sht21_init();
        sensors_vars.sensorsTypes[SENSOR_TEMPERATURE] = 1;
        sensors_vars.sensorsTypes[SENSOR_HUMIDITY] = 1;
    }

    if (max44009_is_present() == 1) {
        max44009_init();
        sensors_vars.sensorsTypes[SENSOR_LIGHT] = 1;
    }

    if (adxl346_is_present() == 1) {
        adxl346_init();
        sensors_vars.sensorsTypes[SENSOR_XACCELERATION] = 1;
        sensors_vars.sensorsTypes[SENSOR_YACCELERATION] = 1;
        sensors_vars.sensorsTypes[SENSOR_ZACCELERATION] = 1;
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
            return &sht21_read_temperature;
        case SENSOR_HUMIDITY:
            return &sht21_read_humidity;
        case SENSOR_LIGHT:
            return &max44009_read_light;
        case SENSOR_XACCELERATION:
            return (callbackRead_cbt) & adxl346_read_x;
        case SENSOR_YACCELERATION:
            return (callbackRead_cbt) & adxl346_read_y;
        case SENSOR_ZACCELERATION:
            return (callbackRead_cbt) & adxl346_read_z;
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
            return &sht21_convert_temperature;
        case SENSOR_HUMIDITY:
            return &sht21_convert_humidity;
        case SENSOR_LIGHT:
            return &max44009_convert_light;
        case SENSOR_XACCELERATION:
            return NULL;
        case SENSOR_YACCELERATION:
            return NULL;
        case SENSOR_ZACCELERATION:
            return NULL;
        case SENSOR_ADCTEMPERATURE:
            return &adc_sens_convert_temperature;
        default:
            return NULL;
    }

}

//=========================== private =========================================

#endif /* BOARD_SENSORS_ENABLED */
