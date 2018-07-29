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
    oscoap_security_context_t  context;
    uint8_t                    path0len;
    uint8_t                    path0val[10];
    uint8_t                    path1len;
    uint8_t                    path1val[10];
} cauthz_access_token_t;

typedef struct {
   coap_resource_desc_t         desc;
   cauthz_access_token_t        accessToken[1];
} cauthz_vars_t;

//=========================== prototypes ======================================

void cauthz_init(void);

/**
\}
\}
*/

#endif
