#ifndef __CJOIN_H
#define __CJOIN_H

/**
\addtogroup AppUdp
\{
\addtogroup cjoin
\{
*/
#include "opendefs.h"
//=========================== define ==========================================
// number of bytes in 802.15.4 short address
#define CJOIN_SHORT_ADDRESS_LENGTH  2
// max number of keys supported in COSE_KeySet
#define CJOIN_MAX_NUM_KEYS          2
// max supported number of bytes in key id field of COSE_Key
#define CJOIN_MAX_KEYID_LENGTH      2
//=========================== typedef =========================================

typedef struct {
    uint8_t*                address;
    asn_t                   lease_asn;
} short_address_t;

typedef struct {
    uint8_t                 kty;
    uint8_t*                kid;
    uint8_t                 kid_len;
    uint8_t*                k;
    uint8_t                 k_len;
} COSE_symmetric_key_t;

typedef struct {
    COSE_symmetric_key_t   key[CJOIN_MAX_NUM_KEYS];
} COSE_keyset_t;

typedef struct {
    uint8_t*                COSE_keyset;
    uint8_t                 COSE_keyset_len;
    uint8_t*                short_address;
    uint8_t                 short_address_len;
} join_response_t;

typedef struct {
   opentimer_id_t           startupTimerId;
   opentimer_id_t           retransmissionTimerId;
   bool                     isJoined;
   asn_t                    joinAsn;
} cjoin_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cjoin_init(void);
void cjoin_schedule(void);
bool cjoin_getIsJoined(void);
void cjoin_setIsJoined(bool newValue);
bool debugPrint_joined(void);

/**
\}
\}
*/

#endif
