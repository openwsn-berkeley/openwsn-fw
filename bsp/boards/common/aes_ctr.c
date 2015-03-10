/**
\brief AES CTR implementation
  
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#include <string.h>
#include <stdint.h>
#include "opendefs.h"
#include "aes_ctr.h"
#include "crypto_engine.h"

static void inc_counter(uint8_t* counter) {
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
owerror_t aes_ctr_enc_raw(uint8_t* buffer, uint8_t len, uint8_t key[16], uint8_t iv[16]) {
   uint8_t n;
   uint8_t k;
   uint8_t nb;
   uint8_t* pbuf;
   uint8_t eiv[16];

   nb = len >> 4;
   for (n = 0; n < nb; n++) {
      pbuf = &buffer[16 * n];
      memcpy(eiv, iv, 16);
      CRYPTO_ENGINE.aes_ecb_enc(eiv, key); 
      // may be faster if vector are aligned to 4 bytes (use long instead char in xor)
      for (k = 0; k < 16; k++) {
         pbuf[k] ^= eiv[k];
      }
      inc_counter(iv);
   }

   return E_SUCCESS;
}

/**
\brief Counter (CTR) mode encryption specific to IEEE 802.15.4E.
\param[in,out] m Pointer to the data that is both authenticated and encrypted. Data is
   overwritten by ciphertext (i.e. plaintext in case of inverse CCM*).
\param[in] len_m Length of data that is both authenticated and encrypted.
\param[in] nonce Buffer containing nonce (13 octets).
\param[in] key Buffer containing the secret key (16 octets).
\param[in,out] mac Buffer containing the unencrypted or encrypted CBC-MAC tag, which depends
   on weather the function is called as part of CCM* forward or inverse transformation. It
   is overwrriten by the encrypted, i.e unencrypted, tag on return.
\param[in] len_mac Length of the CBC-MAC tag. Must be 4, 8 or 16 octets.

\returns E_SUCCESS when the encryption was successful, E_FAIL otherwise. 
*/
owerror_t aes_ctr_enc(uint8_t* m,
         uint8_t len_m,
         uint8_t* nonce,
         uint8_t key[16],
         uint8_t* mac,
         uint8_t len_mac) {

   uint8_t pad_len;
   uint8_t len;
   uint8_t iv[16];
   uint8_t buffer[128 + 16]; // max buffer plus mac

   // asserts here
   if (!((len_mac == 4) || (len_mac == 8) || (len_mac == 16))) {
      return E_FAIL;
   }

   if (len_m > 127) {
      return E_FAIL;
   }

   // iv (flag (1B) | source addr (8B) | ASN (5B) | cnt (2B)
   iv[0] = 0x01;
   memcpy(&iv[1], nonce, 13);
   iv[14] = 0x00;
   iv[15] = 0x00;

   // first block is mac
   memcpy(buffer, mac, len_mac);
   memset(&buffer[len_mac], 0, 16 - len_mac);
   len = 16;

   //  (((x >> 4) + 1)<<4) - x   or    16 - (x % 16) ?
   // m + padding
   pad_len = (((len_m >> 4) + 1) << 4) - len_m;
   pad_len = pad_len == 16 ? 0 : pad_len;
   memcpy(&buffer[len], m, len_m);
   len += len_m;
   memset(&buffer[len], 0, pad_len);
   len += pad_len;

   CRYPTO_ENGINE.aes_ctr_enc_raw(buffer, len, key, iv);

   memcpy(m, &buffer[16], len_m);
   memcpy(mac, buffer, len_mac);

   return E_SUCCESS;
}
