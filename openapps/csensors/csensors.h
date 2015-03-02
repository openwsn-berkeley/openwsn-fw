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

//=========================== define ==========================================

/// task list used for matching callbacks from scheduler to the related timers
#define CSENSORSTASKLIST 40

//=========================== typedef =========================================

typedef struct {
   coap_resource_desc_t       desc;
   uint32_t                   period;
   opentimer_id_t             timerId;
} csensors_resource_t;

typedef struct {
   coap_resource_desc_t       desc;
   uint8_t                    numSensors;
   csensors_resource_t        csensors_resource[MAXSENSORS];
   uint8_t                    cb_list[CSENSORSTASKLIST];
   uint8_t                    cb_put;
   uint8_t                    cb_get;
} csensors_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void csensors_init(void);

/**
\}
\}
*/

#endif
