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
typedef enum {
    COSE_COMMON_HEADER_PARAMETERS_ALG                = 1,
    COSE_COMMON_HEADER_PARAMETERS_CRIT               = 2,
    COSE_COMMON_HEADER_PARAMETERS_CONTENT_TYPE       = 3,
    COSE_COMMON_HEADER_PARAMETERS_KID                = 4,
    COSE_COMMON_HEADER_PARAMETERS_IV                 = 5,
    COSE_COMMON_HEADER_PARAMETERS_PIV                = 6,
    COSE_COMMON_HEADER_PARAMETERS_COUNTER_SIGNATURE  = 7,
} cose_common_header_params_t;
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
   uint8_t                      appKey[16];
} cauthz_vars_t;

//=========================== prototypes ======================================

void cauthz_init(void);

/**
\}
\}
*/

#endif
