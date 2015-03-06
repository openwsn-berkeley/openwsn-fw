/**
\brief AES CBC MAC implementation
  
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#include <string.h>
#include <stdint.h>
#include "opendefs.h"
#include "aes_cbc.h"
#include "crypto_engine.h"

/**
\brief Raw AES-CBC encryption.
\param[in,out] buffer Message to be encrypted. Will be overwritten by ciphertext.
\param[in] len Message length. Must be multiple of 16 octets.
\param[in] key Buffer containing the secret key (16 octets).

\returns E_SUCCESS when the encryption was successful. 
*/
owerror_t aes_cbc_enc_raw(uint8_t* buffer, uint8_t len, uint8_t key[16]) {
   uint8_t  n;
   uint8_t  k;
   uint8_t  nb;
   uint8_t* pbuf;

   nb = len >> 4;
   for (n = 0; n < nb; n++) {
      pbuf = &buffer[16 * n];
      CRYPTO_ENGINE.aes_ecb_enc(pbuf,key);
      if (n < (nb - 1)) {
         // may be faster if vector are aligned to 4 bytes (use long instead char in xor)
         for (k = 0; k < 16; k++) {
            pbuf[16 + k] ^= pbuf[k];
         }
      }
   }

   return E_SUCCESS;
}

/**
\brief CBC-MAC generation specific to IEEE 802.15.4E.
\param[in] a Pointer to the authentication only data.
\param[in] len_a Length of authentication only data.
\param[in] m Pointer to the data that is both authenticated and encrypted.
\param[in] len_m Length of data that is both authenticated and encrypted.
\param[in] saddr Buffer containing source address (8 octets). Used to create a nonce.
\param[in] asn Buffer containing the Absolute Slot Number (5 octets). Used to create a nonce.
\param[in] key Buffer containing the secret key (16 octets).
\param[out] mac Buffer where the value of the CBC-MAC tag will be written.
\param[in] len_mac Length of the CBC-MAC tag. Must be 4, 8 or 16 octets.

\returns E_SUCCESS when the generation was successful, E_FAIL otherwise. 
*/
owerror_t aes_cbc_mac(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t len_m,
         uint8_t saddr[8],
         uint8_t asn[5],
         uint8_t key[16],
         uint8_t* mac,
         uint8_t len_mac) {
   
   uint8_t  pad_len;
   uint8_t  len;
   uint8_t  buffer[128+16]; // max buffer plus IV

   // asserts here
   if (!((len_mac == 4) || (len_mac == 8) || (len_mac == 16))) {
      return E_FAIL;
   }

   if ((len_a > 127) || (len_m > 127) || ((len_a + len_m) > 127)) {
      return E_FAIL;
   }

   if (mac == 0) {
      return E_FAIL;
   }

   // IV: flags (1B) | SADDR (8B) | ASN (5B) | len(m) (2B)
   // X0 xor IV in first 16 bytes of buffer: set buffer[:16] as IV)
   buffer[0] = 0;
   buffer[1] = len_m;
   memcpy(&buffer[2], asn, 5); // assign byte by byte or copy ?
   memcpy(&buffer[7], saddr, 8);
   buffer[15] = 0x49;
   len = 16;

   // len(a)
   buffer[16] = 0;
   buffer[17] = len_a;
   len += 2;

   //  (((x >> 4) + 1)<<4) - x   or    16 - (x % 16) ?
   // a + padding
   pad_len = ((((len_a + 2) >> 4) + 1) << 4) - (len_a + 2);
   pad_len = pad_len == 16 ? 0 : pad_len;
   memcpy(&buffer[len], a, len_a);
   len += len_a;
   memset(&buffer[len], 0, pad_len);
   len += pad_len;

   // m + padding
   pad_len = (((len_m >> 4) + 1) << 4) - len_m;
   pad_len = pad_len == 16 ? 0 : pad_len;
   memcpy(&buffer[len], m, len_m);
   len += len_m;
   memset(&buffer[len], 0, pad_len);
   len += pad_len;

   CRYPTO_ENGINE.aes_cbc_enc_raw(buffer, len, key);

   // copy MAC
   memcpy(mac, &buffer[len - 16], len_mac);

   return E_SUCCESS;
}
