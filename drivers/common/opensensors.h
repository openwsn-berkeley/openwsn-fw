#ifndef __OPENSENSORS_H__
#define __OPENSENSORS_H__

#include "sensors.h"

//=========================== define ==========================================

// Sensor types
enum {
   SENSOR_TEMPERATURE,
   SENSOR_HUMIDITY,
   SENSOR_LIGHT,
   SENSOR_XACCELERATION,
   SENSOR_YACCELERATION,
   SENSOR_ZACCELERATION,
   SENSOR_ADCTEMPERATURE,
   SENSOR_DEFAULT
};

//=========================== typedef =========================================

typedef uint16_t (*callbackRead_cbt)(void);

typedef float (*callbackConvert_cbt)(uint16_t value);

typedef struct {
   uint8_t                          sensorType;
   callbackRead_cbt                 callbackRead;
   callbackConvert_cbt              callbackConvert;
} opensensors_resource_desc_t;

typedef struct {
   opensensors_resource_desc_t      opensensors_resource[NUMSENSORS];
   uint8_t                          numSensors;
} opensensors_vars_t;

//=========================== prototypes ======================================

void opensensors_init(void);
void opensensors_register(
   uint8_t sensorType,
   callbackRead_cbt callbackRead,
   callbackConvert_cbt callbackConvert
);
uint8_t opensensors_getNumSensors(void);
opensensors_resource_desc_t* opensensors_getResource(
    uint8_t index
);

#endif // __OPENSENSORS_H__
