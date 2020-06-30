/**************************************************************
AES128
Author:   Uli Kretzschmar
MSP430 Systems
Freising
AES software support for encryption and decryption
ECCN 5D002 TSU - Technology / Software Unrestricted
Source: http://is.gd/o9RSPq
**************************************************************/
#ifndef OPENWSN_AES128_H
#define OPENWSN_AES128_H

//=========================== prototypes ======================================

/**
\brief Basic AES encryption of a single 16-octet block.
\param[in,out] buffer Single block plaintext. Will be overwritten by ciphertext.
\param[in] key Buffer containing the secret key (16 octets).

\returns E_SUCCESS when the encryption was successful.
*/
owerror_t aes128_enc(uint8_t *buffer, uint8_t *key);

#endif /* OPENWSN_AES128_H */
