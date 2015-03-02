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

opensensors_resource_desc_t* opensensors_get(uint8_t sensorType) {
   uint8_t                          i;
   opensensors_resource_desc_t*     returnResource;
   
   for (i=0;i<opensensors_vars.numSensors;i++) {
      if (opensensors_vars.opensensors_resource[i].sensorType == sensorType) {
         returnResource = &opensensors_vars.opensensors_resource[i];
         break;
      }
   }
   return returnResource;
}


