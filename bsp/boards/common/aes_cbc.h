/**
\brief Definitions for AES CBC MAC implementation

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#ifndef __AES_CBC_H__
#define __AES_CBC_H__

#ifdef  __cplusplus
extern "C" {
#endif

//=========================== prototypes ======================================

owerror_t aes_cbc_mac(uint8_t* a, uint8_t len_a, uint8_t* m, uint8_t len_m, uint8_t* nonce, uint8_t key[16], uint8_t* mac, uint8_t len_mac);
owerror_t aes_cbc_enc_raw(uint8_t* buffer, uint8_t len, uint8_t key[16]);

#ifdef  __cplusplus
}
#endif

#endif /* __AES_CBC_H__ */
