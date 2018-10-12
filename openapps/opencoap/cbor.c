/**
\brief Generic CBOR encoding and decoding functions.
\Author Martin Gunnarsson <martin.gunnarsson@ri.se>
\author Modified by Malisa Vucinic <malishav@gmail.com>
*/

#include "cbor.h"

uint8_t
cbor_dump_text(uint8_t *buffer, const char *text, uint8_t text_len) {
    uint8_t ret = 0;

    if(text_len > 23 ){
        buffer[ret++] = 0x78;
        buffer[ret++] = text_len;
    } else {
        buffer[ret++] = (0x60 | text_len);
    }

    if (text_len != 0 && text != NULL) {
        memcpy(&buffer[ret], text, text_len);
        ret += text_len;
    }

    return ret;
}

uint8_t
cbor_dump_array(uint8_t *buffer, uint8_t elements) {
    uint8_t ret = 0;

    if(elements > 15){
        return 0;
    }

    buffer[ret++] = (0x80 | elements);
    return ret;
}

uint8_t
cbor_dump_bytes(uint8_t *buffer, const uint8_t *bytes, uint8_t bytes_len) {
    uint8_t ret = 0;

    if(bytes_len > 23){
        buffer[ret++] = 0x58;
        buffer[ret++] = bytes_len;
    } else {
        buffer[ret++] = (0x40 | bytes_len);
    }

    if (bytes_len != 0 && bytes != NULL){
        memcpy(&buffer[ret], bytes, bytes_len);
        ret += bytes_len;
    }

    return ret;
}

uint8_t
cbor_dump_unsigned(uint8_t *buffer, uint8_t value) {
    uint8_t ret = 0;

    if(value > 0x17 ){
        buffer[ret++] = 0x18;
        buffer[ret++] = value;
        return ret;
    }

    buffer[ret++] = value;
    return ret;
}

uint8_t
cbor_dump_null(uint8_t *buffer) {
    uint8_t ret = 0;

    buffer[ret++] = 0xf6;
    return ret;
}

uint8_t
cbor_dump_map(uint8_t *buffer, uint8_t elements) {
    uint8_t ret = 0;

    if(elements > 15){
        return 0;
    }

    buffer[ret++] = (0xa0 | elements);
    return ret;
}

/**
\brief Decode a CBOR unsigned integer. Only supports 8-bit values.

This functions attempts to decode a byte string buf into a CBOR unsigned integer.


\param[in] buf The input buffer.
\param[out] value The 8-bit decoded value.
\return Length of the decoded unsigned integer, 0 if error.
*/
uint8_t cbor_load_uint(uint8_t *buf, uint8_t *value) {
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

