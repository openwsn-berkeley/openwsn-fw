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

typedef enum {
   AS_INFO_LABEL_AS      = 0,
   AS_INFO_LABEL_NONCE   = 5,
} as_info_label_t;

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   uint8_t medType;
} cprotected_vars_t;

//=========================== prototypes ======================================

void cprotected_init(void);

/**
\}
\}
*/

#endif
