#ifndef _OPT_CBOR_H
#define _OPT_CBOR_H
#include <stddef.h>
#include <inttypes.h>

uint8_t OPT_CBOR_put_text(uint8_t **buffer, char *text, uint8_t text_len);

uint8_t OPT_CBOR_put_array(uint8_t **buffer,uint8_t elements);

uint8_t OPT_CBOR_put_bytes(uint8_t **buffer, uint8_t bytes_len, uint8_t *bytes);

uint8_t OPT_CBOR_put_map(uint8_t **buffer, uint8_t elements);

uint8_t OPT_CBOR_put_unsigned(uint8_t **buffer, uint8_t value);

#endif /* _OPT_CBOR_H */