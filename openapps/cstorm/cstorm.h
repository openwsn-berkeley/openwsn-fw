#ifndef __CSTORM_H
#define __CSTORM_H

/**
\addtogroup AppUdp
\{
\addtogroup cstorm
\{
*/

#include "coap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimers_id_t     timerId;
   uint16_t             period;   ///< inter-packet period (in ms)
   bool      busySendingCstorm;   ///< TRUE when busy sending cstorm packet
} cstorm_vars_t;

//=========================== prototypes ======================================

void cstorm_init(void);

/**
\}
\}
*/

#endif

