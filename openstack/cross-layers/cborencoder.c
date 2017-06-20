/**
\brief CBOR encoder library.

\author Martin Gunnarsson <martin.gunnarsson@ri.se>
*/


#include "cborencoder.h"

uint8_t cborencoder_put_text(uint8_t **buffer, char *text, uint8_t text_len){
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

uint8_t cborencoder_put_array(uint8_t **buffer,uint8_t elements){
	if(elements > 15){
		return 0;
	}

	**buffer = (0x80 | elements);
	(*buffer)++;
	return 1;
}

uint8_t cborencoder_put_bytes(uint8_t **buffer, uint8_t bytes_len, uint8_t *bytes){
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
uint8_t cborencoder_put_map(uint8_t **buffer, uint8_t elements){
	if(elements > 15){
		return 0;
	}
	**buffer = (0xa0 | elements);
	(*buffer)++;

	return 1;
}

uint8_t cborencoder_put_unsigned(uint8_t **buffer, uint8_t value){
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
