#ifndef __RLEDS_H
#define __RLEDS_H

/**
\addtogroup AppCoAP
\{
\addtogroup netLeds
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rleds_vars_t;

//=========================== prototypes ======================================

void rleds__init();

/**
\}
\}
*/

#endif
