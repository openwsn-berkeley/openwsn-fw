#ifndef __RINFO_H
#define __RINFO_H

/**
\addtogroup AppCoAP
\{
\addtogroup rinfo
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rinfo_vars_t;

//=========================== prototypes ======================================

void rinfo_init();

/**
\}
\}
*/

#endif
