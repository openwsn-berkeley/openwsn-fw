#ifndef __OPENSENSORS_H__
#define __OPENSENSORS_H__

#include "sensors.h"

//=========================== define ==========================================

// Sensor types
enum {
   SENSOR_TEMPERATURE                  = 0x00,
   SENSOR_HUMIDITY                     = 0x01,
   SENSOR_LIGHT                        = 0x02,
   SENSOR_XACCELERATION                = 0x03,
   SENSOR_YACCELERATION                = 0x04,
   SENSOR_ZACCELERATION                = 0x05,
   SENSOR_ADCTEMPERATURE               = 0x06,
};

#define MAXSENSORS           10

//=========================== typedef =========================================

typedef uint16_t (*callbackRead_cbt)(void);

typedef float (*callbackConvert_cbt)(uint16_t value);

typedef struct {
   uint8_t                   sensorType;
   callbackRead_cbt          callbackRead;
   callbackConvert_cbt       callbackConvert;
} opensensors_resource_desc_t;

typedef struct {
   opensensors_resource_desc_t   opensensors_resource[MAXSENSORS];
   uint8_t                   numSensors;
} opensensors_vars_t;

//=========================== prototypes ======================================

void opensensors_init(void);
void opensensors_register(
   uint8_t sensorType,
   callbackRead_cbt callbackRead,
   callbackConvert_cbt callbackConvert
);
opensensors_resource_desc_t* opensensors_get(uint8_t sensorType);

#endif // __OPENSENSORS_H__
