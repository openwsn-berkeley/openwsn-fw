/**
\brief Definitions for AES CBC MAC implementation

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#ifndef __CBC_MAC_H__
#define __CBC_MAC_H__

#ifdef  __cplusplus
extern "C" {
#endif

int aes_cbc_mac_enc(uint8_t* a, uint8_t len_a, uint8_t* m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t* key, uint8_t* mac, uint8_t len_mac);
int aes_cbc_mac_enc_raw(uint8_t* buffer, uint8_t len, uint8_t key[16]);

#ifdef  __cplusplus
}
#endif

#endif /* __CBC_MAC_H__ */
