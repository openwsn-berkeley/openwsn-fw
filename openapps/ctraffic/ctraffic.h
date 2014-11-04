#ifndef __CTRAFFIC_H
#define __CTRAFFIC_H

/**
\addtogroup AppUdp
\{
\addtogroup ctraffic
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t       timerId;
   uint16_t             period;   ///< inter-packet period (in ms)
} ctraffic_vars_t;

//=========================== prototypes ======================================

void ctraffic_init(void);

/**
\}
\}
*/

#endif
