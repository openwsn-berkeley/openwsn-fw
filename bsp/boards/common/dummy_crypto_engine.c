/**
\brief Implementation of a dummy crypto engine that does nothing

\author Malisa Vucinic <malishav@gmail.com>
*/

#include <stdint.h>
#include <string.h>
#include "dummy_crypto_engine.h"

static int aes_cbc_mac_enc_raw_identity(uint8_t *buffer, uint8_t len, uint8_t key[16]) {
   return 0;
}

static int aes_cbc_mac_enc_identity(uint8_t *a,
   uint8_t len_a,
   uint8_t *m,
   uint8_t len_m,
   uint8_t saddr[8],
   uint8_t asn[5],
   uint8_t *key,
   uint8_t *mac,
   uint8_t len_mac) {

   return 0;
}

static int aes_ccms_enc_identity(uint8_t *a,
   uint8_t len_a,
   uint8_t *m,
   uint8_t *len_m,
   uint8_t saddr[8],
   uint8_t asn[5],
   uint8_t *key) {
   
   return 0;
}

static int aes_ccms_dec_identity(uint8_t *a,
   uint8_t len_a,
   uint8_t *m,
   uint8_t *len_m,
   uint8_t saddr[8],
   uint8_t asn[5],
   uint8_t *key) {
   
   return 0;
}

static int aes_ctr_enc_raw_identity(uint8_t *buffer, uint8_t len, uint8_t *key, uint8_t iv[16]) {
   return 0;
}

static int aes_ctr_enc_identity(uint8_t *m,
   uint8_t len_m,
   uint8_t saddr[8],
   uint8_t asn[5],
   uint8_t *key,
   uint8_t *mac,
   uint8_t len_mac) {
   
   return 0;
}

static int aes_ecb_enc_identity(uint8_t *buffer, uint8_t *key) {
   return 0;
}

static int init(void)
{
   return 0;
}

/*---------------------------------------------------------------------------*/
const struct crypto_engine dummy_crypto_engine = {
   aes_ccms_enc_identity,
   aes_ccms_dec_identity,
   aes_cbc_mac_enc_identity,
   aes_cbc_mac_enc_raw_identity,
   aes_ctr_enc_identity,
   aes_ctr_enc_raw_identity,
   aes_ecb_enc_identity,
   init,
};
/*---------------------------------------------------------------------------*/

