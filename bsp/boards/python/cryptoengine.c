/**
\brief Wrapper of software implementation of CCM.

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/

#include <stdint.h>
#include <string.h>
#include "cryptoengine.h"

owerror_t cryptoengine_aes_ccms_enc(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   
   return aes_ccms_enc(a,len_a, m, len_m, nonce, l, key, len_mac);
}

owerror_t cryptoengine_aes_ccms_dec(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   
   return aes_ccms_dec(a, len_a, m, len_m, nonce, l, key, len_mac);
}

owerror_t cryptoengine_aes_ecb_enc(uint8_t* buffer, uint8_t* key) {
   return aes_ecb_enc(buffer, key);
}

owerror_t cryptoengine_init(void) {
   return E_SUCCESS;
}

