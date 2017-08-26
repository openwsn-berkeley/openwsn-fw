/**
\brief Implementation of a crypto engine that does nothing

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/

#include <stdint.h>
#include <string.h>
#include "dummy_crypto_engine.h"

static owerror_t aes_cbc_enc_raw_identity(uint8_t* buffer, uint8_t len, uint8_t key[16], uint8_t iv[16]) {
   return E_SUCCESS;
}

static owerror_t aes_ccms_enc_identity(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   
   return E_SUCCESS;
}

static owerror_t aes_ccms_dec_identity(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   
   return E_SUCCESS;
}

static owerror_t aes_ctr_enc_raw_identity(uint8_t* buffer, uint8_t len, uint8_t key[16], uint8_t iv[16]) {
   return E_SUCCESS;
}

static owerror_t aes_ecb_enc_identity(uint8_t* buffer, uint8_t* key) {
   return E_SUCCESS;
}

static owerror_t init(void) {
   return E_SUCCESS;
}

/*---------------------------------------------------------------------------*/
const struct crypto_engine dummy_crypto_engine = {
   aes_ccms_enc_identity,
   aes_ccms_dec_identity,
   aes_cbc_enc_raw_identity,
   aes_ctr_enc_raw_identity,
   aes_ecb_enc_identity,
   init,
};
/*---------------------------------------------------------------------------*/

