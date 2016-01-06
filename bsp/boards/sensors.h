/**
    \brief Declaration of the "sensors" board-specif driver.
    \author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
*/

#ifndef __SENSORS_H__
#define __SENSORS_H__

//=========================== define ==========================================

/// define NUMSENSORS if not defined in board_info.h
#ifndef NUMSENSORS
#define NUMSENSORS 10
#endif  // NUMSENSORS

/// Sensor types
enum {
   SENSOR_ZERO,                 // new sensor types go after this one
   SENSOR_TEMPERATURE,
   SENSOR_HUMIDITY,
   SENSOR_LIGHT,
   SENSOR_XACCELERATION,
   SENSOR_YACCELERATION,
   SENSOR_ZACCELERATION,
   SENSOR_ADCTEMPERATURE,
   SENSOR_DEFAULT,
   SENSOR_LAST                  // new sensor types go before this one
};

//=========================== typedef =========================================

typedef uint16_t (*callbackRead_cbt)(void);

typedef float (*callbackConvert_cbt)(uint16_t value);

typedef struct {
   uint8_t      sensorsTypes[SENSOR_LAST];
} sensors_vars_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

void sensors_init(void);
bool sensors_is_present(uint8_t sensorType);
callbackRead_cbt sensors_getCallbackRead(uint8_t sensorType);
callbackConvert_cbt sensors_getCallbackConvert(uint8_t sensorType);

#endif // __SENSORS_H__
