/**
\brief Definitions for AES CCMS implementation

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#ifndef __AES_CCMS_H__
#define __AES_CCMS_H__

#ifdef  __cplusplus
extern "C" {
#endif

//=========================== prototypes ======================================

owerror_t firmware_aes_ccms_enc(uint8_t* a,uint8_t len_a,uint8_t* m,uint8_t* len_m,uint8_t* nonce, uint8_t l, uint8_t key[16], uint8_t len_mac);
owerror_t firmware_aes_ccms_dec(uint8_t* a,uint8_t len_a,uint8_t* m,uint8_t* len_m,uint8_t* nonce, uint8_t l, uint8_t key[16], uint8_t len_mac);

#ifdef  __cplusplus
}
#endif

#endif /* __AES_CCMS_H__ */
