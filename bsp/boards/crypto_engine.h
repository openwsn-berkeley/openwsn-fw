/**
\brief Defitions for crypto engine initialization

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>
*/
#ifndef __CRYPTO_ENGINE_H__
#define __CRYPTO_ENGINE_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "opendefs.h"

#define CBC_MAC_SIZE  4

#ifdef CRYPTO_ENGINE_SCONS
#define CRYPTO_ENGINE CRYPTO_ENGINE_SCONS
#else /* CRYPTO_ENGINE_SCONS */
#define CRYPTO_ENGINE dummy_crypto_engine
#endif /* CRYPTO_ENGINE_SCONS */

typedef int (*fp_aes_ccms_enc)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t *len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key);
typedef int (*fp_aes_ccms_dec)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t *len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key);
typedef int (*fp_aes_cbc_mac_enc)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key, uint8_t *mac, uint8_t len_mac);
typedef int (*fp_aes_cbc_mac_enc_raw)(uint8_t *buffer, uint8_t len, uint8_t key[16]);
typedef int (*fp_aes_ctr_enc)(uint8_t *m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key, uint8_t *mac, uint8_t len_mac);
typedef int (*fp_aes_ctr_enc_raw)(uint8_t *buffer, uint8_t len, uint8_t *key, uint8_t iv[16]);
typedef int (*fp_aes_ecb_enc)(uint8_t *buffer, uint8_t *key);
typedef int (*fp_init)(void);

struct crypto_engine
{
    fp_aes_ccms_enc aes_ccms_enc;
    fp_aes_ccms_dec aes_ccms_dec;
    fp_aes_cbc_mac_enc aes_cbc_mac_enc;
    fp_aes_cbc_mac_enc_raw aes_cbc_mac_enc_raw;
    fp_aes_ctr_enc aes_ctr_enc;
    fp_aes_ctr_enc_raw aes_ctr_enc_raw;
    fp_aes_ecb_enc aes_ecb_enc;
    fp_init init;
};

extern const struct crypto_engine CRYPTO_ENGINE;

#ifdef  __cplusplus
}
#endif

#endif /* __CRYPTO_ENGINE_H__ */
