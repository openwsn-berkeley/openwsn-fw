#ifndef __CEXAMPLE_H
#define __CEXAMPLE_H

/**
\addtogroup AppUdp
\{
\addtogroup cexample
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t       timerId;
} cexample_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cexample_init(void);

/**
\}
\}
*/

#endif
