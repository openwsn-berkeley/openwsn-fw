#ifndef OPENWSN_CSTORM_H
#define OPENWSN_CSTORM_H

/**
\addtogroup AppUdp
\{
\addtogroup cstorm
\{
*/

#include "config.h"
#include "coap.h"
#include "opentimers.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    coap_resource_desc_t desc;
    opentimers_id_t timerId;
    uint16_t period;   ///< inter-packet period (in ms)
    bool busySendingCstorm;   ///< TRUE when busy sending cstorm packet
} cstorm_vars_t;

//=========================== prototypes ======================================

void cstorm_init(void);

/**
\}
\}
*/

#endif /* OPENWSN_CSTORM_H */

