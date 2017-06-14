/**
\brief AES CBC implementation
  
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/
#include <string.h>
#include <stdint.h>
#include "opendefs.h"
#include "aes_cbc.h"
#include "cryptoengine.h"

/**
\brief Raw AES-CBC encryption.
\param[in,out] buffer Message to be encrypted. Will be overwritten by ciphertext.
\param[in] len Message length. Must be multiple of 16 octets.
\param[in] key Buffer containing the secret key (16 octets).
\param[in] iv Buffer containing the Initialization Vector (16 octets).

\returns E_SUCCESS when the encryption was successful. 
*/
owerror_t aes_cbc_enc_raw(uint8_t* buffer, uint8_t len, uint8_t key[16], uint8_t iv[16]) {
   uint8_t  n;
   uint8_t  k;
   uint8_t  nb;
   uint8_t* pbuf;
   uint8_t* pxor;

   nb = len >> 4;
   pxor = iv;
   for (n = 0; n < nb; n++) {
      pbuf = &buffer[16 * n];
      // may be faster if vector are aligned to 4 bytes (use long instead char in xor)
      for (k = 0; k < 16; k++) {
            pbuf[k] ^= pxor[k];
      }
      cryptoengine_aes_ecb_enc(pbuf,key);
      pxor = pbuf;
   }
   return E_SUCCESS;
}


