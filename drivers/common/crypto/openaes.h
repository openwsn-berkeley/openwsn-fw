/**************************************************************
AES128
Author:   Uli Kretzschmar
MSP430 Systems
Freising
AES software support for encryption and decryption
ECCN 5D002 TSU - Technology / Software Unrestricted
Source: http://is.gd/o9RSPq
**************************************************************/
#ifndef __OPENAES_H__
#define __OPENAES_H__

//=========================== prototypes ======================================

owerror_t openaes_enc(uint8_t* buffer, uint8_t* key);

#endif /* __OPENAES_H__ */
