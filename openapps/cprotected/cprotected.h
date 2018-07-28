#ifndef __CPROTECTED_H
#define __CPROTECTED_H

/**
\addtogroup AppCoAP
\{
\addtogroup cprotected
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} cprotected_vars_t;

//=========================== prototypes ======================================

void cprotected_init(void);

/**
\}
\}
*/

#endif
