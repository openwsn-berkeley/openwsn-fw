#ifndef __CBENCHMARK_H
#define __CBENCHMARK_H

/**
\addtogroup AppUdp
\{
\addtogroup cbenchmark
\{
*/
#include "opendefs.h"
//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
    coap_resource_desc_t     desc;
    opentimers_id_t          timerId;
} cbenchmark_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cbenchmark_init(void);

/**
\}
\}
*/

#endif
