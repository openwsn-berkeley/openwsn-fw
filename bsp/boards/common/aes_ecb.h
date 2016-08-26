/**************************************************************
AES128
Author:   Uli Kretzschmar
MSP430 Systems
Freising
AES software support for encryption and decryption
ECCN 5D002 TSU - Technology / Software Unrestricted
Source: http://is.gd/o9RSPq
**************************************************************/
#ifndef __AES_ECB_H__
#define __AES_ECB_H__

#ifdef  __cplusplus
extern "C" {
#endif

//=========================== prototypes ======================================

owerror_t aes_ecb_enc(uint8_t* buffer, uint8_t* key);

#ifdef  __cplusplus
}
#endif

#endif /* __AES_ECB_H__ */
