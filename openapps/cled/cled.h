#ifndef __CLEDS_H
#define __CLEDS_H

/**
\addtogroup AppCoAP
\{
\addtogroup cled
\{
*/

#include "coap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} cled_vars_t;

//=========================== prototypes ======================================

void cled_init(void);

/**
\}
\}
*/

#endif
