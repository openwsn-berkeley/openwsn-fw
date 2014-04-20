#ifndef __RRT_H
#define __RRT_H

/**
\addtogroup AppCoAP
\{
\addtogroup rrt
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rrt_vars_t;

//=========================== prototypes ======================================

void rrt_init();

/**
\}
\}
*/

#endif
