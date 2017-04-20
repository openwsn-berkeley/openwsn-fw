/*e
\brief A minimal CBOR parser implementation of draft-6tisch-minimal-security-02.
*/
#include "cbor.h"
//=========================== defines =========================================
// number of bytes in 802.15.4 short address
#define SHORT_ADDRESS_LENGTH    2
#define ASN_LENGTH              5
//=========================== variables =======================================

//=========================== prototypes ======================================
owerror_t cbor_parse_keyset(COSE_keyset_t *, uint8_t *, uint8_t *);
owerror_t cbor_parse_short_address(short_address_t *, uint8_t *, uint8_t *);
owerror_t cbor_parse_key(COSE_symmetric_key_t *, uint8_t *, uint8_t *);


//=========================== public ==========================================

/**
\brief Parse the received join response.

This function expects the join response structure from minimal-security-02 draft.

\param[out] response The join_response_t structure containing parsed info.
\param[in] buf The received join response.
\param[in] len Length of the payload.
*/
owerror_t cbor_parse_join_response(join_response_t *response, uint8_t *buf, uint8_t len) {

    cbor_majortype_t major_type;
    uint8_t additional_info;
    uint8_t ret;
    uint8_t *tmp;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    additional_info = *buf & CBOR_ADDINFO_MASK;
        
    if (major_type != CBOR_MAJORTYPE_ARRAY) {
        return E_FAIL;
    }

    if (additional_info > 2 || additional_info == 0) {
        return E_FAIL;  // unsupported join response structure
    }

    tmp++;

    if (cbor_parse_keyset(&(response->keyset), tmp, &ret) == E_FAIL) {
        return E_FAIL;
    }

    tmp += ret;
    
    if (additional_info == 2) { // short address present
        if (cbor_parse_short_address(&(response->short_address), tmp, &ret) == E_FAIL) {
            return E_FAIL;
        }
        tmp += ret;
    }

    if ( (uint8_t)(tmp - buf) != len) { // final check that everything has been parsed 
        memset(response, 0x00, sizeof(join_response_t)); // invalidate join response
        return E_FAIL;
    }

    return E_SUCCESS;
}


//=========================== private =========================================

/**
\brief Parse the received COSE_Keyset.

The function expects COSE_Keyset with symmetric keys as per minimal-security-02 draft
and parses it into COSE_symmetric_key_t structure.

\param[out] keyset The COSE_keyset_t structure containing parsed keys.
\param[in] buf Input buffer.
\param[out] len Processed length.
*/
owerror_t cbor_parse_keyset(COSE_keyset_t *keyset, uint8_t *buf, uint8_t* len) {

    cbor_majortype_t major_type;
    uint8_t additional_info;
    uint8_t i;
    uint8_t ret;
    uint8_t *tmp;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    additional_info = *buf & CBOR_ADDINFO_MASK;
        
    if (major_type != CBOR_MAJORTYPE_ARRAY) {
        return E_FAIL;
    }

    if (additional_info > COSE_KEYSET_MAX_NUM_KEYS || additional_info == 0) {
        return E_FAIL;  // unsupported join response structure
    }

    tmp++;

    for(i = 0; i < additional_info; i++) {
        // parse symmetric key map
        if (cbor_parse_key(&keyset->key[i], tmp, &ret) == E_FAIL) {
            return E_FAIL;
        }
        tmp += ret;
    }

    *len = (uint8_t) (tmp - buf);
    return E_SUCCESS;
}

/**
\brief Parse a COSE symmetric key.

The function expects COSE symmetric key as per minimal-security-02 draft
and parses it into COSE_symmetric_key_t structure.

\param[out] key The COSE_symmetric_ket_t structure containing parsed key.
\param[in] buf Input buffer.
\param[out] len Processed length.
*/
owerror_t cbor_parse_key(COSE_symmetric_key_t *key, uint8_t* buf, uint8_t* len) {

    cbor_majortype_t major_type;
    uint8_t additional_info;
    uint8_t i;
    uint8_t *tmp;
    uint8_t l;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    additional_info = *buf & CBOR_ADDINFO_MASK;
        
    if (major_type != CBOR_MAJORTYPE_MAP) {
        return E_FAIL;
    }

    if (additional_info > COSE_SYMKEY_MAXNUMPAIRS || additional_info == 0) {
        return E_FAIL;  // unsupported join response structure
    }

    tmp++;

    for (i = 0; i < additional_info; i++) {
        switch((cose_key_label_t) *tmp) {
            case COSE_KEY_LABEL_KTY:
                tmp++;
                key->kty = (cose_key_value_t) *tmp;
                tmp++;
                break;
            case COSE_KEY_LABEL_ALG: // step by key alg
                tmp++;
                tmp++;
                break;
            case COSE_KEY_LABEL_KID:
                tmp++;
                l = *tmp & CBOR_ADDINFO_MASK;
                tmp++;
                key->kid = tmp;
                key->kid_len = l;
                tmp += l;
                break;
            case COSE_KEY_LABEL_K:
                tmp++;
                l = *tmp & CBOR_ADDINFO_MASK;
                tmp++;
                key->k = tmp;
                key->k_len = l;
                tmp += l;
                break;
            case COSE_KEY_LABEL_BASEIV:   // step by base iv
                tmp++;
                l = *tmp & CBOR_ADDINFO_MASK;
                tmp++;
                tmp += l;
                break;
            case COSE_KEY_LABEL_KEYOPS: // step by key ops
                tmp++;
                l = *tmp & CBOR_ADDINFO_MASK;
                tmp++;
                tmp += l;
                break;
            default:
                return E_FAIL;
        }
    }

    *len = (uint8_t) (tmp - buf);
    return E_SUCCESS;
}

/**
\brief Parse the received short address.

The function expects short_address as per minimal-security-02 draft
and parses it into short_address_t structure.

\param[out] address The short_address_t structure containing parsed short_address and lease time.
\param[in] buf Input buffer.
\param[out] len Processed length.
*/
owerror_t cbor_parse_short_address(short_address_t *short_address, uint8_t *buf, uint8_t* len) {
    
    cbor_majortype_t major_type;
    uint8_t additional_info;
    uint8_t *tmp;
    uint8_t l;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    additional_info = *buf & CBOR_ADDINFO_MASK;
        
    if (major_type != CBOR_MAJORTYPE_ARRAY) {
        return E_FAIL;
    }

    if (additional_info > 2 || additional_info == 0) {
        return E_FAIL;  // unsupported join response structure
    }

    tmp++;
    l = *tmp & CBOR_ADDINFO_MASK;
    
    if (l != SHORT_ADDRESS_LENGTH) {
        return E_FAIL;
    }

    tmp++;
    short_address->address = tmp;

    tmp += l;

    if (additional_info == 2) { // lease time present
        l = *tmp & CBOR_ADDINFO_MASK;
        if (l != ASN_LENGTH) { // 5 byte ASN expected
            return E_FAIL;
        }
        tmp++;

        (short_address->lease_asn).bytes0and1           = ((uint16_t) tmp[1] << 8) | ((uint16_t) tmp[0]);
        (short_address->lease_asn).bytes2and3           = ((uint16_t) tmp[3] << 8) | ((uint16_t) tmp[2]);
        (short_address->lease_asn).byte4                = tmp[4]; 

        tmp += ASN_LENGTH;
    }

    *len = (uint8_t) (tmp - buf);
    return E_SUCCESS;
}

