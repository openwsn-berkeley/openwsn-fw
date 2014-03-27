#ifndef __RWELLKNOWN_H
#define __RWELLKNOWN_H

/**
\addtogroup AppCoAP
\{
\addtogroup rWellKnown
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rwellknown_vars_t;

//=========================== prototypes ======================================

void rwellknown_init();

/**
\}
\}
*/

#endif
