#ifndef _CBOR_H
#define _CBOR_H

#include "opendefs.h"

uint8_t cbor_dump_text(uint8_t *buffer, const char *text, uint8_t text_len);
uint8_t cbor_dump_null(uint8_t *buffer);
uint8_t cbor_dump_unsigned(uint8_t *buffer, uint8_t value);
uint8_t cbor_dump_bytes(uint8_t *buffer, const uint8_t *bytes, uint8_t bytes_len);
uint8_t cbor_dump_array(uint8_t *buffer, uint8_t elements);
uint8_t cbor_dump_map(uint8_t *buffer, uint8_t elements);

#endif /* _CBOR_H */
