#ifndef __UDPSTORM_H
#define __UDPSTORM_H

/**
\addtogroup AppUdp
\{
\addtogroup udpStorm
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
} udpstorm_vars_t;

//=========================== prototypes ======================================

void udpstorm_init(void);

/**
\}
\}
*/

#endif
