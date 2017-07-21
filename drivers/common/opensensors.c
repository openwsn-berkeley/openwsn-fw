/**
\brief Definition of the "opensensors" driver.

\author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
*/

#include "opendefs.h"
#include "opensensors.h"
#include "sensors.h"

//=========================== defines =========================================

//=========================== variables =======================================

opensensors_vars_t opensensors_vars;

//=========================== prototypes ======================================

void opensensors_register(
   uint8_t sensorType
);

//=========================== public ==========================================

/**
   \brief Initialize OpenSensors
*/
void opensensors_init(void) {
   uint8_t sensorType;
   
   memset(&opensensors_vars,0,sizeof(opensensors_vars_t));
   for (sensorType=SENSOR_ZERO+1;sensorType<SENSOR_LAST;sensorType++) {
      opensensors_register(sensorType);
   }
}

/**
   \brief Return the number of registered OpenSensors
   \returns the number of registered OpenSensors.
*/
uint8_t opensensors_getNumSensors(void) {
   return opensensors_vars.numSensors;
}

/**
   \brief Return an OpenSensors resource structure
   \param[in] index Index of the OpenSensors resource within Opensensors vars.
   \returns the OpenSensors resource within Opensensors vars.
*/
opensensors_resource_desc_t* opensensors_getResource(uint8_t index) {
   return &(opensensors_vars.opensensors_resource[index]);
}

//=========================== private =========================================

/**
   \brief Register a OpenSensors resource

   \param[in] sensorType the sensor type representation.
*/
void opensensors_register(uint8_t sensorType) {
   if (sensors_is_present(sensorType)) {
      opensensors_vars.opensensors_resource[opensensors_vars.numSensors].sensorType      = sensorType;
      opensensors_vars.opensensors_resource[opensensors_vars.numSensors].callbackRead    = sensors_getCallbackRead(sensorType);
      opensensors_vars.opensensors_resource[opensensors_vars.numSensors].callbackConvert = sensors_getCallbackConvert(sensorType);
      opensensors_vars.numSensors++;
   }
}
