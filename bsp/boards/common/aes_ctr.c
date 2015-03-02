/**
  \brief AES CTR implementation
  
  \author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>
 */
#include <string.h>
#include <stdint.h>
#include "crypto_driver.h"

static void inc_counter(uint8_t *counter) 
{
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

int aes_ctr_enc_raw(uint8_t *buffer, uint8_t len, uint8_t *key, uint8_t iv[16])
{
   uint8_t n, k, nb, *pbuf;
   uint8_t eiv[16];

   const crypto_driver_t* drv = crypto_driver_get();

   nb = len >> 4;
   for (n = 0; n < nb; n++) {
      pbuf = &buffer[16 * n];
      memcpy(eiv, iv, 16);
      drv->aes_ecb_enc(eiv, key); 
      // may be faster if vector are aligned to 4 bytes (use long instead char in xor)
      for (k = 0; k < 16; k++){
         pbuf[k] ^= eiv[k];
      }
      inc_counter(iv);
   }

   return 0;

}

int aes_ctr_enc(uint8_t *m,
      uint8_t len_m,
      uint8_t saddr[8],
      uint8_t asn[5],
      uint8_t *key,
      uint8_t *mac,
      uint8_t len_mac)
{
   uint8_t pad_len;
   uint8_t len;
   uint8_t iv[16];
   uint8_t buffer[128 + 16]; // max buffer plus mac

   const crypto_driver_t* drv = crypto_driver_get();

   // asserts here
   if (!((len_mac == 4) || (len_mac == 8) || (len_mac == 16)))
      return -1;

   if (len_m > 127)
      return -2;

   // iv (flag (1B) | source addr (8B) | ASN (5B) | cnt (2B)
   iv[0] = 0x01;
   memcpy(&iv[1], saddr, 8);
   memcpy(&iv[9], asn, 5); // assign byte by byte or copy ?
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

   drv->aes_ctr_enc_raw(buffer, len, key, iv);

   memcpy(m, &buffer[16], len_m);
   memcpy(mac, buffer, len_mac);

   return 0;
}
