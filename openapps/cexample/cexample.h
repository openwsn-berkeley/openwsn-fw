#ifndef __CEXAMPLE_H
#define __CEXAMPLE_H

/**
\addtogroup AppUdp
\{
\addtogroup cexample
\{
*/

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

#endif

