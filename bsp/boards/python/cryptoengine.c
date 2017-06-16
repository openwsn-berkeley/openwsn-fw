/**
\brief Wrapper of software implementation of CCM.

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/

#include <stdint.h>
#include <string.h>
#include "cryptoengine_obj.h"
#include "openccms_obj.h"
#include "openaes_obj.h"

owerror_t cryptoengine_aes_ccms_enc(OpenMote *self,
         uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   
   return openccms_enc(self, a,len_a, m, len_m, nonce, l, key, len_mac);

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
    return openccms_dec(self, a, len_a, m, len_m, nonce, l, key, len_mac);
}

owerror_t cryptoengine_aes_ecb_enc(OpenMote* self, uint8_t* buffer, uint8_t* key) {
    return openaes_enc(self, buffer, key);
}

owerror_t cryptoengine_init(OpenMote *self) {
    return E_SUCCESS;
}

