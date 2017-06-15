/**
\brief Wrapper of software implementation of CCM.

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/

#include <stdint.h>
#include <string.h>
#include "cryptoengine_obj.h"
#include "firmware_aes_ccms_obj.h"
#include "firmware_aes_ecb_obj.h"

owerror_t cryptoengine_aes_ccms_enc(OpenMote *self,
         uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   
   return firmware_aes_ccms_enc(self, a,len_a, m, len_m, nonce, l, key, len_mac);

}

owerror_t cryptoengine_aes_ccms_dec(OpenMote* self,
         uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
    return firmware_aes_ccms_dec(self, a, len_a, m, len_m, nonce, l, key, len_mac);
}

owerror_t cryptoengine_aes_ecb_enc(OpenMote* self, uint8_t* buffer, uint8_t* key) {
    return firmware_aes_ecb_enc(self, buffer, key);
}

owerror_t cryptoengine_init(OpenMote *self) {
    return E_SUCCESS;
}

