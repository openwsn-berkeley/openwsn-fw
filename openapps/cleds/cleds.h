#ifndef __CLEDS_H
#define __CLEDS_H

/**
\addtogroup AppCoAP
\{
\addtogroup cleds
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} cleds_vars_t;

//=========================== prototypes ======================================

void cleds__init(void);

/**
\}
\}
*/

#endif
