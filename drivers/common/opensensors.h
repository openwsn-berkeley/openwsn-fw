/**
    \brief Declaration of the "opensensors" driver.
    \author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
*/

#ifndef OPENWSN_OPENSENSORS_H
#define OPENWSN_OPENSENSORS_H


/**
\addtogroup drivers
\{
\addtogroup OpenSensors
\{
*/

#include "sensors.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
    uint8_t sensorType;
    callbackRead_cbt callbackRead;
    callbackConvert_cbt callbackConvert;
} opensensors_resource_desc_t;

//=========================== module variables ================================

typedef struct {
    opensensors_resource_desc_t opensensors_resource[NUMSENSORS];
    uint8_t numSensors;
} opensensors_vars_t;

//=========================== prototypes ======================================

void opensensors_init(void);

uint8_t opensensors_getNumSensors(void);

opensensors_resource_desc_t *opensensors_getResource(
        uint8_t index
);

/**
\}
\}
*/

#endif /* OPENWSN_OPENSENSORS_H */
