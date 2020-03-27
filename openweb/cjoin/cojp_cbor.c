/*e
\brief CBOR helper functions implementing decoding and encoding of structures defined in draft-6tisch-minimal-security-06.
*/
#include "cojp_cbor.h"
#include "cborencoder.h"

//=========================== defines =========================================
// number of bytes in 802.15.4 short address
#define ASN_LENGTH              5
//=========================== variables =======================================

//=========================== prototypes ======================================
owerror_t cojp_cbor_decode_link_layer_keyset(uint8_t *, uint8_t *, cojp_link_layer_keyset_t *);
owerror_t cojp_cbor_decode_link_layer_short_address(uint8_t *, uint8_t *, cojp_link_layer_short_address_t *);
owerror_t cojp_cbor_decode_ipv6_address(uint8_t *, uint8_t *, open_addr_t *);

uint8_t cbor_decode_uint(uint8_t *buf, uint8_t *value);

//=========================== public ==========================================
/**
\brief Parse the CoJP Configuration object.

The function expects COSE symmetric key as per minimal-security-02 draft
and parses it into COSE_symmetric_key_t structure.

\param[in] buf Input buffer.
\param[out] len Processed length.
\param[out] configuration The cojp_configuration_object_t structure containing parsed parameters.
*/
owerror_t cojp_cbor_decode_configuration_object(uint8_t *buf, uint8_t len, cojp_configuration_object_t *configuration) {

    cbor_majortype_t major_type;
    uint8_t num_elems;
    uint8_t ret;
    uint8_t i;
    uint8_t *tmp;
    uint8_t error;

    memset(configuration, 0x00, sizeof(cojp_configuration_object_t));
    ret = 0;
    error = 0;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    num_elems = *buf & CBOR_ADDINFO_MASK;

    if (major_type != CBOR_MAJORTYPE_MAP) {
        return E_FAIL;
    }

    if (num_elems > COJP_CONFIGURATION_MAX_NUM_PARAMS) {
        return E_FAIL;  // unsupported configuration object structure
    }

    tmp++;

    for (i = 0; i < num_elems; i++) {
        switch((cojp_parameters_labels_t) *tmp) {
            case COJP_PARAMETERS_LABELS_LLKEYSET:
                tmp++;
                if (cojp_cbor_decode_link_layer_keyset(tmp, &ret, &(configuration->keyset)) == E_FAIL) {
                    error++;
                }
                tmp += ret;
                break;
            case COJP_PARAMETERS_LABELS_LLSHORTADDRESS:
                if (cojp_cbor_decode_link_layer_short_address(tmp, &ret, &(configuration->short_address)) == E_FAIL) {
                    error++;
                }
                tmp += ret;
                break;
            case COJP_PARAMETERS_LABELS_JRCADDRESS:
                if (cojp_cbor_decode_ipv6_address(tmp, &ret, &(configuration->jrc_address)) == E_FAIL) {
                    error++;
                }
                tmp += ret;
                break;
            default:
                // FIXME skip the unsupported parameter and go to the next one
                error++;
        }
    }

    if ( (uint8_t)(tmp - buf) != len) { // final check that everything has been processed
        error++;
    }

    if (error) {
        memset(configuration, 0x00, sizeof(cojp_configuration_object_t));
        return E_FAIL;
    }

    return E_SUCCESS;
}

/**
\brief Encode a Join_Request object.

This functions encodes the cojp_join_request_object_t structure into a byte string.


\param[out] buf The output buffer.
\param[in] join_request The cojp_join_request_object_t data structure containing the Join_Request parameters.
\return Length of the encoded object..
*/
uint8_t cojp_cbor_encode_join_request_object(uint8_t *buf, cojp_join_request_object_t *join_request) {
    uint8_t len;
    uint8_t elements;

    len = 0;
    elements = 0;

    if (join_request->role == COJP_ROLE_VALUE_6N) {
        elements = 1;
    } else {
        elements = 2;
    }

    len += cborencoder_put_map(&buf[len], elements);
    if (elements == 2) {
        len += cborencoder_put_unsigned(&buf[len], (uint8_t) COJP_PARAMETERS_LABELS_ROLE);
        len += cborencoder_put_unsigned(&buf[len], (uint8_t) join_request->role);
    }
    len += cborencoder_put_unsigned(&buf[len], (uint8_t) COJP_PARAMETERS_LABELS_NETID);
    len += cborencoder_put_bytes(&buf[len], (join_request->pan_id)->panid, LENGTH_ADDR16b);

    return len;
}

//=========================== private =========================================

/**
\brief Decode a Link-Layer Keyset object.

The function expects Link-Layer Keyset object as defined in draft-ietf-6tisch-minimal-security-06
and parses it into a cojp_link_layer_keyset_t structure.

\param[in] buf Input buffer.
\param[out] len Processed length.
\param[out] keyset The cojp_link_layer_keyset_t structure containing the parsed keys.
\return E_SUCCESS if all elements are successfully processed, E_FAIL in all other cases.
*/
owerror_t cojp_cbor_decode_link_layer_keyset(uint8_t *buf, uint8_t* len, cojp_link_layer_keyset_t *keyset) {

    cbor_majortype_t major_type;
    uint8_t add_info;
    uint8_t i;
    uint8_t l;
    uint8_t *tmp;
    uint8_t tmp_key_usage; // 8-bit variant of key usage
    uint8_t ret;
    cojp_link_layer_key_t *current_key;
    uint8_t current_key_index;

    current_key_index = 0;
    i = 0;
    ret = 0;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    add_info = *buf & CBOR_ADDINFO_MASK;

    // assert
    if (major_type != CBOR_MAJORTYPE_ARRAY) {
        return E_FAIL;
    }

    // sanity check
    if (add_info < COJP_KEY_MIN_NUM_ELEMS) {   // additional info of an array is the number of elements
        return E_FAIL;
    }

    tmp++;

    while(i < add_info) { // while there are elements left in the array

        if (current_key_index >= KEYSET_MAX_NUM_KEYS) {
            return E_FAIL; // more keys than we can handle
        }

        major_type = (cbor_majortype_t) *tmp >> 5;

        if (major_type == CBOR_MAJORTYPE_UINT) {

            current_key = (cojp_link_layer_key_t *) &(keyset->key[current_key_index]);

            ret = cbor_decode_uint(tmp, &current_key->key_index);
            tmp += ret;
            i++; // moving on to the next element

            major_type = (cbor_majortype_t) *tmp >> 5;
            l = *tmp & CBOR_ADDINFO_MASK;

            if (major_type == CBOR_MAJORTYPE_UINT) { // optional key usage as a uint is present
                ret = cbor_decode_uint(tmp, &tmp_key_usage);
                current_key->key_usage = (cojp_key_usage_values_t) tmp_key_usage;
                tmp += ret;
                i++;

                major_type = (cbor_majortype_t) *tmp >> 5;
                l = *tmp & CBOR_ADDINFO_MASK;
            } else { // key usage is not present, implies the default value
                current_key->key_usage = COJP_KEY_USAGE_6TiSCH_K1K2_ENC_MIC32;
            }

            if (major_type == CBOR_MAJORTYPE_BSTR) { // mandatory key_value parameter as a bstr
                 if (l != AES128_KEY_LENGTH) {
                     return E_FAIL; // unsupported
                 }
                 tmp++;
                 memcpy(current_key->key_value, tmp, AES128_KEY_LENGTH);
                 tmp += l;
                 i++; // moving on to the next element, if any
            } else { // last element in the group is not a bstr -> error
                return E_FAIL;
            }
        } else { // first element in the group is not a uint -> error
            return E_FAIL;
        }

        current_key_index++;
    }

    keyset->num_keys = current_key_index;
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
\param[out] short_address The cojp_link_layer_short_address_t structure containing the parsed short address.
\return E_SUCCESS in case of success, E_FAIL otherwise.
*/
owerror_t cojp_cbor_decode_link_layer_short_address(uint8_t *buf, uint8_t *len, cojp_link_layer_short_address_t *short_address) {

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
        return E_FAIL;  // unsupported Link-Layer Short Address object
    }

    tmp++;
    major_type = (cbor_majortype_t) *buf >> 5;
    l = *tmp & CBOR_ADDINFO_MASK;

    if (major_type != CBOR_MAJORTYPE_BSTR) { // first element is not a bst -> error
        return E_FAIL;
    }

    if (l != IEEE802154_SHORT_ADDRESS_LENGTH) { // length of the bstr is not what IEEE802.15.4 expects
        return E_FAIL;
    }

    tmp++;
    memcpy(short_address->address, tmp, IEEE802154_SHORT_ADDRESS_LENGTH);

    tmp += l;

    if (additional_info == 2) { // lease time present
        // TODO lease_time parsing unsupported for now, do nothing
    }

    *len = (uint8_t) (tmp - buf);
    return E_SUCCESS;
}

/**
\brief Decodes a CBOR bstr to an IPv6 address.

This functions attempts to decode a byte string buf into an IPv6 address, as an open_addr_t structure.

\param[in] buf The input buffer.
\param[out] len The processed length.
\param[out] ipv6_address The open_addr_t structure containing the parsed IPv6 address.
\return E_SUCCESS in case of success, E_FAIL otherwise.
*/
owerror_t cojp_cbor_decode_ipv6_address(uint8_t *buf, uint8_t *len, open_addr_t *ipv6_address) {
    uint8_t major_type;
    uint8_t add_info;
    uint8_t *tmp;

    major_type = (cbor_majortype_t) *buf >> 5;
    add_info = *buf & CBOR_ADDINFO_MASK;
    tmp = buf;

    if (major_type != CBOR_MAJORTYPE_BSTR) { // Does not decode into a bstr -> error
        return E_FAIL;
    }

    if (add_info != LENGTH_ADDR128b) {
        return E_FAIL; // length of the bstr is different than the length of an IPv6 address (16) - > error
    }

    tmp++;

    ipv6_address->type = ADDR_128B;
    memcpy(ipv6_address->addr_128b, tmp, LENGTH_ADDR128b);

    *len = (uint8_t) (tmp - buf);;
    return E_SUCCESS;
}

/**
\brief Decode a CBOR unsigned integer. Only supports 8-bit values.

This functions attempts to decode a byte string buf into a CBOR unsigned integer.


\param[in] buf The input buffer.
\param[out] value The 8-bit decoded value.
\return Length of the decoded unsigned integer, 0 if error.
*/
uint8_t cbor_decode_uint(uint8_t *buf, uint8_t *value) {
    uint8_t major_type;
    uint8_t add_info;

    major_type = (cbor_majortype_t) *buf >> 5;
    add_info = *buf & CBOR_ADDINFO_MASK;

    // assert
    if (major_type != CBOR_MAJORTYPE_UINT) {
        return 0;
    }

    if (add_info < 23) {
        *value = add_info;
        return 0 + 1;
    } else if (add_info == 24) { // uint8_t follows
        *value = buf[1];
        return 1 + 1;
    } else if (add_info == 25) { // uint16_t follows
        return 1 + 2;
    } else if (add_info == 26) { // uint32_t follows
        return 1 + 4;
    } else if (add_info == 27) { // uint64_t follows
        return 1 + 8;
    }

    return 0;
}

