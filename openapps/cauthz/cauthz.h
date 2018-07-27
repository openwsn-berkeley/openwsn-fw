#ifndef __CAUTHZ_H
#define __CAUTHZ_H

/**
\addtogroup AppCoAP
\{
\addtogroup cauthz
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} cauthz_vars_t;

//=========================== prototypes ======================================

void cauthz_init(void);

/**
\}
\}
*/

#endif
