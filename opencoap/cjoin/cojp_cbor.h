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
#define KEYSET_MAX_NUM_KEYS               2
// CBOR additional info mask
#define CBOR_ADDINFO_MASK                 0x1F
// symmetric key length
#define AES128_KEY_LENGTH                 16
// IEEE 802.15.4 key id length
#define IEEE802154_KEYID_LENGTH           1
// IEEE 802.15.4 short address
#define IEEE802154_SHORT_ADDRESS_LENGTH   2
// CoJP Configuration object constants
#define COJP_CONFIGURATION_MAX_NUM_PARAMS 3 // TODO we don't support 6LBR joining for now
// CoJP Min num elements in a key
#define COJP_KEY_MIN_NUM_ELEMS            2
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

typedef enum {
    COJP_PARAMETERS_LABELS_ROLE           = 1, // Identifies the role parameter
    COJP_PARAMETERS_LABELS_LLKEYSET       = 2, // Identifies the array carrying one or more link-layer cryptographic keys
    COJP_PARAMETERS_LABELS_LLSHORTADDRESS = 3, // Identifies the assigned link-layer short address
    COJP_PARAMETERS_LABELS_JRCADDRESS     = 4, // Identifies the IPv6 address of the JRC
    COJP_PARAMETERS_LABELS_NETID          = 5, // Identifies the network identifier (PAN ID)
    COJP_PARAMETERS_LABELS_NETPREFIX      = 6, // Identifies the IPv6 prefix of the network
} cojp_parameters_labels_t;

typedef enum {
    COJP_ROLE_VALUE_6N                    = 0, // 6TiSCH Node
    COJP_ROLE_VALUE_6LBR                  = 1, // 6LBR Node
} cojp_role_values_t;

typedef enum {
    COJP_KEY_USAGE_6TiSCH_K1K2_ENC_MIC32  = 0,
    COJP_KEY_USAGE_6TiSCH_K1K2_ENC_MIC64  = 1,
    COJP_KEY_USAGE_6TiSCH_K1K2_ENC_MIC128 = 2,
    COJP_KEY_USAGE_6TiSCH_K1K2_MIC32      = 3,
    COJP_KEY_USAGE_6TiSCH_K1K2_MIC64      = 4,
    COJP_KEY_USAGE_6TiSCH_K1K2_MIC128     = 5,
    COJP_KEY_USAGE_6TiSCH_K1_MIC32        = 6,
    COJP_KEY_USAGE_6TiSCH_K1_MIC64        = 7,
    COJP_KEY_USAGE_6TiSCH_K1_MIC128       = 8,
    COJP_KEY_USAGE_6TiSCH_K2_MIC32        = 9,
    COJP_KEY_USAGE_6TiSCH_K2_MIC64        = 10,
    COJP_KEY_USAGE_6TiSCH_K2_MIC128       = 11,
    COJP_KEY_USAGE_6TiSCH_K2_ENC_MIC32    = 12,
    COJP_KEY_USAGE_6TiSCH_K2_ENC_MIC64    = 13,
    COJP_KEY_USAGE_6TiSCH_K2_ENC_MIC128   = 14,
} cojp_key_usage_values_t;

typedef struct {
    uint8_t                         address[IEEE802154_SHORT_ADDRESS_LENGTH];
    uint32_t                        lease_time;
} cojp_link_layer_short_address_t;

typedef struct {
    uint8_t                         key_index;
    cojp_key_usage_values_t         key_usage;
    uint8_t                         key_value[AES128_KEY_LENGTH];
} cojp_link_layer_key_t;

typedef struct {
    uint8_t                         num_keys;
    cojp_link_layer_key_t           key[KEYSET_MAX_NUM_KEYS];
} cojp_link_layer_keyset_t;

typedef struct {
    cojp_link_layer_keyset_t        keyset;
    cojp_link_layer_short_address_t short_address;
    open_addr_t                     jrc_address;
} cojp_configuration_object_t;

typedef struct {
    cojp_role_values_t              role;
    open_addr_t *                   pan_id;
} cojp_join_request_object_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

owerror_t cojp_cbor_decode_configuration_object(uint8_t *buf, uint8_t len, cojp_configuration_object_t *configuration);
uint8_t cojp_cbor_encode_join_request_object(uint8_t *buf, cojp_join_request_object_t *join_request);

/**
\}
\}
*/

#endif

