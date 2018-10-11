/**
\brief CBOR encoding functions.
\Author Martin Gunnarsson <martin.gunnarsson@ri.se>
\author Modified by Malisa Vucinic <malishav@gmail.com>
*/

#include "cborencoder.h"

uint8_t
cborencoder_put_text(uint8_t *buffer, const char *text, uint8_t text_len) {
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
cborencoder_put_array(uint8_t *buffer, uint8_t elements) {
    uint8_t ret = 0;

    if(elements > 15){
        return 0;
    }

    buffer[ret++] = (0x80 | elements);
    return ret;
}

uint8_t
cborencoder_put_bytes(uint8_t *buffer, const uint8_t *bytes, uint8_t bytes_len) {
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
cborencoder_put_unsigned(uint8_t *buffer, uint8_t value) {
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
cborencoder_put_null(uint8_t *buffer) {
    uint8_t ret = 0;

    buffer[ret++] = 0xf6;
    return ret;
}

uint8_t
cborencoder_put_map(uint8_t *buffer, uint8_t elements) {
    uint8_t ret = 0;

    if(elements > 15){
        return 0;
    }

    buffer[ret++] = (0xa0 | elements);
    return ret;
}

