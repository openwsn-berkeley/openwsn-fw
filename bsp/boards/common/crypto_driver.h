/**
\brief Defitions for crypto driver initialization

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>
*/
#ifndef __CRYPTO_DRIVER_H__
#define __CRYPTO_DRIVER_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "opendefs.h"

#define CBC_MAC_SIZE  4

typedef int (*fp_aes_ccms_enc)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t *len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key);
typedef int (*fp_aes_ccms_dec)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t *len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key);
typedef int (*fp_aes_cbc_mac_enc)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key, uint8_t *mac, uint8_t len_mac);
typedef int (*fp_aes_cbc_mac_enc_raw)(uint8_t *buffer, uint8_t len, uint8_t key[16]);
typedef int (*fp_aes_ctr_enc)(uint8_t *m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key, uint8_t *mac, uint8_t len_mac);
typedef int (*fp_aes_ctr_enc_raw)(uint8_t *buffer, uint8_t len, uint8_t *key, uint8_t iv[16]);
typedef int (*fp_aes_ecb_enc)(uint8_t *buffer, uint8_t *key);

typedef struct crypto_driver_s
{
    fp_aes_ccms_enc aes_ccms_enc;
    fp_aes_ccms_dec aes_ccms_dec;
    fp_aes_cbc_mac_enc aes_cbc_mac_enc;
    fp_aes_cbc_mac_enc_raw aes_cbc_mac_enc_raw;
    fp_aes_ctr_enc aes_ctr_enc;
    fp_aes_ctr_enc_raw aes_ctr_enc_raw;
    fp_aes_ecb_enc aes_ecb_enc;
} crypto_driver_t;

int crypto_driver_init(void);
const crypto_driver_t* crypto_driver_get(void);

#ifdef  __cplusplus
}
#endif

#endif /* __CRYPTO_DRIVER_H__ */
