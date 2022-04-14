#ifndef OPENWSN_CINFO_H
#define OPENWSN_CINFO_H

/**
\addtogroup AppCoAP
\{
\addtogroup cinfo
\{
*/

#include "config.h"
#include "coap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    coap_resource_desc_t desc;
} cinfo_vars_t;

//=========================== prototypes ======================================

void cinfo_init(void);

/**
\}
\}
*/

#endif /* OPENWSN_CINFO_H */
