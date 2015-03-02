#ifndef __CSENSORS_H
#define __CSENSORS_H

/**
\addtogroup AppCoAP
\{
\addtogroup csensors
\{
*/

#include "opencoap.h"
#include "opensensors.h"
#include "sensors.h"

//=========================== define ==========================================

/// task list used for matching callbacks from scheduler to the related timers
#define CSENSORSTASKLIST 40

//=========================== typedef =========================================

typedef struct {
   coap_resource_desc_t         desc;
   opensensors_resource_desc_t* opensensors_resource;
   uint16_t                     period;
   opentimer_id_t               timerId;
} csensors_resource_t;

typedef struct {
   coap_resource_desc_t         desc;
   csensors_resource_t          csensors_resource[NUMOPENSENSORS];
   uint8_t                      numCsensors;
   uint8_t                      cb_list[CSENSORSTASKLIST];
   uint8_t                      cb_put;
   uint8_t                      cb_get;
} csensors_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void csensors_init(void);

/**
\}
\}
*/

#endif
