/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:22.623421.
*/
/*
 * AES module by Texas Instruments
 */

#ifndef TI_AES
#define TI_AES

uint8_t Key[64];

void aes_encrypt(unsigned char *input,unsigned char *state, unsigned char *key);
//void aes_decrypt(unsigned char *state, unsigned char *key);

#endif
