
#include "opt-cbor.h"
#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTF_HEX(data, len) 	oscoap_printf_hex(data, len)
#else
#define PRINTF(...)
#define PRINTF_HEX(data, len)
#endif



uint8_t OPT_CBOR_put_text(uint8_t **buffer, char *text, uint8_t text_len){
	uint8_t ret = 0;

	if(text_len > 23 ){
		**buffer = 0x78;
		(*buffer)++;
		**buffer = text_len;
		(*buffer)++;
		ret += 2;
	}else{
		**buffer = (0x60 | text_len);
		(*buffer)++;
		ret += 1;
	}

	memcpy(*buffer, text, text_len);
	(*buffer)+= text_len;
	ret += text_len;
	return ret;
}

uint8_t OPT_CBOR_put_array(uint8_t **buffer,uint8_t elements){
	if(elements > 15){
		PRINTF("ERROR! in put array\n");
		return 0;
	}

	**buffer = (0x80 | elements);
	(*buffer)++;
	return 1;
}

uint8_t OPT_CBOR_put_bytes(uint8_t **buffer, uint8_t bytes_len, uint8_t *bytes){
	uint8_t ret = 0;
	if(bytes_len > 23){
		**buffer = 0x58;
		(*buffer)++;
		**buffer = bytes_len;
		(*buffer)++;
		ret += 2;
	}else{
		**buffer = (0x40 | bytes_len);
		(*buffer)++;
		ret += 1;
	}
	memcpy(*buffer, bytes, bytes_len);
	(*buffer) += bytes_len;
	ret += bytes_len;
	return ret;
}
uint8_t OPT_CBOR_put_map(uint8_t **buffer, uint8_t elements){
	if(elements > 15){
		PRINTF("ERROR in put map\n");
		return 0;
	}
	**buffer = (0xa0 | elements);
	(*buffer)++;

	return 1;
}

uint8_t OPT_CBOR_put_unsigned(uint8_t **buffer, uint8_t value){
	if(value > 0x17 ){
			(**buffer) = (0x18);
			(*buffer)++;
			(**buffer) = (value);
			(*buffer)++;
		return 2;
	}
	(**buffer) = (value);
	(*buffer)++;
	return 1;
}