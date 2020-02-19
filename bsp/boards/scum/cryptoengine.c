/**
\brief Dummy implementation of cryptoengine.
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
   
   return E_FAIL;
}

owerror_t cryptoengine_aes_ccms_dec(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   
   return E_FAIL;
}

owerror_t cryptoengine_aes_ecb_enc(uint8_t* buffer, uint8_t* key) {
    // To be implemented
    return E_FAIL;
}

owerror_t cryptoengine_init(void) {
    // To be implemented
    return E_SUCCESS;
}

