/**
  \brief AES CCMS implementation
  
  \author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
 */
#include <string.h>
#include <stdint.h>
#include "crypto_engine.h"

int aes_ccms_enc(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t saddr[8],
         uint8_t asn[5],
         uint8_t* key) {

   uint8_t mac[CBC_MAC_SIZE];

   if (CRYPTO_ENGINE.aes_cbc_mac_enc(a, len_a, m, *len_m, saddr, asn, key, mac, CBC_MAC_SIZE) == 0) {
      if (CRYPTO_ENGINE.aes_ctr_enc(m, *len_m, saddr, asn, key, mac, CBC_MAC_SIZE) == 0) {
         memcpy(&m[*len_m], mac, CBC_MAC_SIZE);
         *len_m += CBC_MAC_SIZE;

         return 0;
      }
   }

   return -1;
}

int aes_ccms_dec(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t saddr[8],
         uint8_t asn[5],
         uint8_t* key) {

   uint8_t mac[CBC_MAC_SIZE];
   uint8_t orig_mac[CBC_MAC_SIZE];

   *len_m -= CBC_MAC_SIZE;
   memcpy(mac, &m[*len_m], CBC_MAC_SIZE);

   if (CRYPTO_ENGINE.aes_ctr_enc(m, *len_m, saddr, asn, key, mac, CBC_MAC_SIZE) == 0) {
      if (CRYPTO_ENGINE.aes_cbc_mac_enc(a, len_a, m, *len_m, saddr, asn, key, orig_mac, CBC_MAC_SIZE) == 0) {
         if (memcmp(mac, orig_mac, CBC_MAC_SIZE) == 0) {
            return 0;
         }
      }
   }

   return -1;
}
