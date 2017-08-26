#ifndef __CWELLKNOWN_H
#define __CWELLKNOWN_H

/**
\addtogroup AppCoAP
\{
\addtogroup cwellknown
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} cwellknown_vars_t;

//=========================== prototypes ======================================

void cwellknown_init(void);

/**
\}
\}
*/

#endif
