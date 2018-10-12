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
// symmetric key length
#define AES128_KEY_LENGTH                 16

//=========================== typedef =========================================

typedef enum {
    COSE_COMMON_HEADER_PARAMETERS_ALG                = 1,
    COSE_COMMON_HEADER_PARAMETERS_CRIT               = 2,
    COSE_COMMON_HEADER_PARAMETERS_CONTENT_TYPE       = 3,
    COSE_COMMON_HEADER_PARAMETERS_KID                = 4,
    COSE_COMMON_HEADER_PARAMETERS_IV                 = 5,
    COSE_COMMON_HEADER_PARAMETERS_PIV                = 6,
    COSE_COMMON_HEADER_PARAMETERS_COUNTER_SIGNATURE  = 7,
} cose_common_header_parameters_labels_t;

typedef enum {
    COSE_KEY_LABEL_KTY                          = 1,      // RFC8152
    COSE_KEY_LABEL_KID                          = 2,      // RFC8152
    COSE_KEY_LABEL_ALG                          = 3,      // RFC8152
    COSE_KEY_LABEL_KEYOPS                       = 4,      // RFC8152
    COSE_KEY_LABEL_BASEIV                       = 5,      // RFC8152
    COSE_KEY_LABEL_K                            = 32,     // (-1) RFC8152
    COSE_KEY_LABEL_CLIENT_ID                    = 6,      // draft-ietf-ace-oscore-profile-02
    COSE_KEY_LABEL_SERVER_ID                    = 7,      // draft-ietf-ace-oscore-profile-02
    COSE_KEY_LABEL_KDF                          = 8,      // draft-ietf-ace-oscore-profile-02
    COSE_KEY_LABEL_SLT                          = 9,      // draft-ietf-ace-oscore-profile-02
} cose_key_parameters_labels_t;

typedef enum {
    COSE_KEY_VALUE_OKP                          = 1,     // RFC8152
    COSE_KEY_VALUE_EC2                          = 2,     // RFC8152
    COSE_KEY_VALUE_SYMMETRIC                    = 4,     // RFC8152
} cose_key_values_t;

typedef enum {
    ACE_PARAMETERS_LABELS_AUD                   = 3,
    ACE_PARAMETERS_LABELS_CLIENT_ID             = 8,
    ACE_PARAMETERS_LABELS_CLIENT_SECRET         = 9,
    ACE_PARAMETERS_LABELS_RESPONSE_TYPE         = 10,
    ACE_PARAMETERS_LABELS_REDIRECT_URI          = 11,
    ACE_PARAMETERS_LABELS_SCOPE                 = 12,
    ACE_PARAMETERS_LABELS_STATE                 = 13,
    ACE_PARAMETERS_LABELS_CODE                  = 14,
    ACE_PARAMETERS_LABELS_ERROR                 = 15,
    ACE_PARAMETERS_LABELS_ERROR_DESCRIPTION     = 16,
    ACE_PARAMETERS_LABELS_ERROR_URI             = 17,
    ACE_PARAMETERS_LABELS_GRANT_TYPE            = 18,
    ACE_PARAMETERS_LABELS_ACCESS_TOKEN          = 19,
    ACE_PARAMETERS_LABELS_TOKEN_TYPE            = 20,
    ACE_PARAMETERS_LABELS_EXPIRES_IN            = 21,
    ACE_PARAMETERS_LABELS_USERNAME              = 22,
    ACE_PARAMETERS_LABELS_PASSWORD              = 23,
    ACE_PARAMETERS_LABELS_REFRESH_TOKEN         = 24,
    ACE_PARAMETERS_LABELS_CNF                   = 25,
    ACE_PARAMETERS_LABELS_PROFILE               = 26,
    ACE_PARAMETERS_LABELS_RS_CNF                = 31,
} ace_parameters_labels_t;

typedef enum {
    ACE_CWT_CNF_COSE_KEY                        = 1,
    ACE_CWT_CNF_ENCRYPTED_COSE_KEY              = 2,
    ACE_CWT_CNF_KID                             = 3,
} ace_cwt_cnf_cose_key_t;

//=========================== variables =======================================
typedef struct {
    oscoap_security_context_t  context;
    uint8_t                    path0len;
    uint8_t                    path0val[10];
    uint8_t                    path1len;
    uint8_t                    path1val[10];
} cauthz_oscore_cwt_t;

typedef struct {
   coap_resource_desc_t         desc;
   cauthz_oscore_cwt_t          accessToken;
   uint8_t                      appKey[16];
} cauthz_vars_t;

//=========================== prototypes ======================================

void cauthz_init(void);

/**
\}
\}
*/

#endif
