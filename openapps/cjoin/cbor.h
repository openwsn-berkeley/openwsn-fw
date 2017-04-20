#ifndef __CBOR_H
#define __CBOR_H

/**
\addtogroup AppUdp
\{
\addtogroup cjoin
\{
*/
#include "opendefs.h"
//=========================== define ==========================================

// max number of keys supported in COSE_KeySet
#define COSE_KEYSET_MAX_NUM_KEYS          2
// CBOR additional info mask
#define CBOR_ADDINFO_MASK                 0x1F
// max number of pairs in COSE symmetric key struct
#define COSE_SYMKEY_MAXNUMPAIRS           6    

//=========================== typedef =========================================

// CBOR major types
typedef enum {
    CBOR_MAJORTYPE_UINT                 = 0,
    CBOR_MAJORTYPE_NINT                 = 1,
    CBOR_MAJORTYPE_BSTR                 = 2,
    CBOR_MAJORTYPE_TSTR                 = 3,
    CBOR_MAJORTYPE_ARRAY                = 4,
    CBOR_MAJORTYPE_MAP                  = 5,
} cbor_majortype_t;

// COSE key map labels
typedef enum {
    COSE_KEY_LABEL_KTY                  = 1,
    COSE_KEY_LABEL_KID                  = 2,
    COSE_KEY_LABEL_ALG                  = 3,
    COSE_KEY_LABEL_KEYOPS               = 4,
    COSE_KEY_LABEL_BASEIV               = 5,
    COSE_KEY_LABEL_K                    = 32, // -1
} cose_key_label_t;

// COSE key type values
typedef enum {
    COSE_KEY_VALUE_OKP                  = 1,
    COSE_KEY_VALUE_EC2                  = 2,
    COSE_KEY_VALUE_SYMMETRIC            = 4,
} cose_key_value_t;

typedef struct {
    uint8_t*                address;
    asn_t                   lease_asn;
} short_address_t;

typedef struct {
    cose_key_value_t        kty;
    uint8_t*                kid;
    uint8_t                 kid_len;
    uint8_t*                k;
    uint8_t                 k_len;
} COSE_symmetric_key_t;

typedef struct {
    COSE_symmetric_key_t   key[COSE_KEYSET_MAX_NUM_KEYS];
} COSE_keyset_t;

typedef struct {
    COSE_keyset_t           keyset;
    short_address_t         short_address;
} join_response_t;


//=========================== variables =======================================

//=========================== prototypes ======================================

owerror_t cjoin_parse_join_response(join_response_t *, uint8_t *, uint8_t);

/**
\}
\}
*/

#endif

