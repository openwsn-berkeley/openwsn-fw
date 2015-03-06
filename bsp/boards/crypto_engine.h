/**
\brief Definitions for crypto engine initialization

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#ifndef __CRYPTO_ENGINE_H__
#define __CRYPTO_ENGINE_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "opendefs.h"

//=========================== define ==========================================
#define CBC_MAC_SIZE  4

#ifdef CRYPTO_ENGINE_SCONS
#define CRYPTO_ENGINE CRYPTO_ENGINE_SCONS
#else /* CRYPTO_ENGINE_SCONS */
#define CRYPTO_ENGINE dummy_crypto_engine
#endif /* CRYPTO_ENGINE_SCONS */

//=========================== typedef =========================================

typedef owerror_t (*fp_aes_ccms_enc)(uint8_t* a, uint8_t len_a, uint8_t* m, uint8_t* len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t* key);
typedef owerror_t (*fp_aes_ccms_dec)(uint8_t* a, uint8_t len_a, uint8_t* m, uint8_t* len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t* key);
typedef owerror_t (*fp_aes_cbc_mac_enc)(uint8_t* a, uint8_t len_a, uint8_t* m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t* key, uint8_t* mac, uint8_t len_mac);
typedef owerror_t (*fp_aes_cbc_mac_enc_raw)(uint8_t* buffer, uint8_t len, uint8_t key[16]);
typedef owerror_t (*fp_aes_ctr_enc)(uint8_t* m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t* key, uint8_t* mac, uint8_t len_mac);
typedef owerror_t (*fp_aes_ctr_enc_raw)(uint8_t* buffer, uint8_t len, uint8_t* key, uint8_t iv[16]);
typedef owerror_t (*fp_aes_ecb_enc)(uint8_t* buffer, uint8_t* key);
typedef owerror_t (*fp_init)(void);

//=========================== module variables ================================

struct crypto_engine {
    fp_aes_ccms_enc           aes_ccms_enc;              // CCM* encryption
    fp_aes_ccms_dec           aes_ccms_dec;              // CCM* decryption
    fp_aes_cbc_mac_enc        aes_cbc_mac_enc;           // CBC-MAC generation
    fp_aes_cbc_mac_enc_raw    aes_cbc_mac_enc_raw;       // CBC encryption
    fp_aes_ctr_enc            aes_ctr_enc;               // 15.4-specific CTR encryption
    fp_aes_ctr_enc_raw        aes_ctr_enc_raw;           // CTR encryption
    fp_aes_ecb_enc            aes_ecb_enc;               // AES block encryption
    fp_init                   init;                      // Init function
};

extern const struct crypto_engine CRYPTO_ENGINE;

#ifdef  __cplusplus
}
#endif

#endif /* __CRYPTO_ENGINE_H__ */
