/**
\brief AES CTR implementation
  
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#include <string.h>
#include <stdint.h>
#include "opendefs.h"
#include "firmware_aes_ctr.h"
#include "cryptoengine.h"

static void firmware_inc_counter(uint8_t* counter) {
   // from openssl
   uint32_t n = 16;
   uint8_t  c;
   do {
      --n;
      c = counter[n];
      ++c;
      counter[n] = c;
      if (c) return;
   } while (n);
}

/**
\brief Raw AES-CTR encryption.
\param[in,out] buffer Message to be encrypted. Will be overwritten by ciphertext.
\param[in] len Message length. Must be multiple of 16 octets.
\param[in] key Buffer containing the secret key (16 octets).
\param[in] iv Buffer containing the Initialization Vector (16 octets).

\returns E_SUCCESS when the encryption was successful. 
*/
owerror_t firmware_aes_ctr_enc_raw(uint8_t* buffer, uint8_t len, uint8_t key[16], uint8_t iv[16]) {
   uint8_t n;
   uint8_t k;
   uint8_t nb;
   uint8_t* pbuf;
   uint8_t eiv[16];

   nb = len >> 4;
   for (n = 0; n < nb; n++) {
      pbuf = &buffer[16 * n];
      memcpy(eiv, iv, 16);
      cryptoengine_aes_ecb_enc(eiv, key); 
      // may be faster if vector are aligned to 4 bytes (use long instead char in xor)
      for (k = 0; k < 16; k++) {
         pbuf[k] ^= eiv[k];
      }
      firmware_inc_counter(iv);
   }

   return E_SUCCESS;
}

