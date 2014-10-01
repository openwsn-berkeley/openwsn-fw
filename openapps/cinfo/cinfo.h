#ifndef __CINFO_H
#define __CINFO_H

/**
\addtogroup AppCoAP
\{
\addtogroup cinfo
\{
*/

#include "opencoap.h"

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

#endif
