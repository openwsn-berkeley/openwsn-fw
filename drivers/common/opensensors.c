#include "opendefs.h"
#include "opensensors.h"

opensensors_vars_t opensensors_vars;

void opensensors_init(void) {
   memset(&opensensors_vars,0,sizeof(opensensors_vars_t));
}

void opensensors_register(uint8_t path1len,
                      uint8_t* path1val,
                      callbackRead_cbt callbackRead,
                      callbackConvert_cbt callbackConvert) {
   opensensors_vars.opensensors_resource[opensensors_vars.numSensors].path1len        = path1len;
   opensensors_vars.opensensors_resource[opensensors_vars.numSensors].path1val        = path1val;
   opensensors_vars.opensensors_resource[opensensors_vars.numSensors].callbackRead    = callbackRead;
   opensensors_vars.opensensors_resource[opensensors_vars.numSensors].callbackConvert = callbackConvert;
   opensensors_vars.numSensors++;
}

opensensors_vars_t* opensensors_read(void) {
   return &opensensors_vars;
}


