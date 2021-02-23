#ifndef OPENWSN_CEXAMPLE_H
#define OPENWSN_CEXAMPLE_H

/**
\addtogroup AppUdp
\{
\addtogroup cexample
\{
*/

#include "opentimers.h"
#include "config.h"
#include "coap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
    coap_resource_desc_t desc;
    opentimers_id_t timerId;
    bool busySendingCexample;
} cexample_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cexample_init(void);

/**
\}
\}
*/

#endif /* OPENWSN_CEXAMPLE_H */

