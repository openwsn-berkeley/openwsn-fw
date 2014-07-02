/*
 * AES module by Texas Instruments
 */

#ifndef TI_AES
#define TI_AES

uint8_t Key[64];

void aes_encrypt(unsigned char *input,unsigned char *state, unsigned char *key);

#endif
