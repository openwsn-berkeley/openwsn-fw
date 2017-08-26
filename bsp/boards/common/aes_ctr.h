/**
\brief Definitions for AES CTR implementation

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#ifndef __AES_CTR_H__
#define __AES_CTR_H__

#ifdef  __cplusplus
extern "C" {
#endif

//=========================== prototypes ======================================

owerror_t aes_ctr_enc_raw(uint8_t* buffer, uint8_t len, uint8_t key[16], uint8_t iv[16]);

#ifdef  __cplusplus
}
#endif

#endif /* __AES_CTR_H__ */
