#ifndef _CBOR_H
#define _CBOR_H

#include "opendefs.h"

//=========================== define ==========================================

// CBOR additional info mask
#define CBOR_ADDINFO_MASK                 0x1F

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

//=========================== variables =======================================

//=========================== prototypes ======================================

uint8_t cbor_dump_text(uint8_t *buffer, const char *text, uint8_t text_len);
uint8_t cbor_dump_null(uint8_t *buffer);
uint8_t cbor_dump_unsigned(uint8_t *buffer, uint8_t value);
uint8_t cbor_dump_bytes(uint8_t *buffer, const uint8_t *bytes, uint8_t bytes_len);
uint8_t cbor_dump_array(uint8_t *buffer, uint8_t elements);
uint8_t cbor_dump_map(uint8_t *buffer, uint8_t elements);

uint8_t cbor_load_uint(uint8_t *buf, uint8_t *value);

#endif /* _CBOR_H */
