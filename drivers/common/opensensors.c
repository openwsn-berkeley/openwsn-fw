#include "opendefs.h"
#include "opensensors.h"

opensensors_vars_t opensensors_vars;

void opensensors_init(void) {
   memset(&opensensors_vars,0,sizeof(opensensors_vars_t));
}

void opensensors_register(uint8_t sensorType,
                      callbackRead_cbt callbackRead,
                      callbackConvert_cbt callbackConvert) {
   opensensors_vars.opensensors_resource[opensensors_vars.numSensors].sensorType      = sensorType;
   opensensors_vars.opensensors_resource[opensensors_vars.numSensors].callbackRead    = callbackRead;
   opensensors_vars.opensensors_resource[opensensors_vars.numSensors].callbackConvert = callbackConvert;
   opensensors_vars.numSensors++;
}

uint8_t opensensors_getNumSensors(void) {
   return opensensors_vars.numSensors;
}

opensensors_resource_desc_t* opensensors_getResource(uint8_t index) {
   return &(opensensors_vars.opensensors_resource[index]);
}
