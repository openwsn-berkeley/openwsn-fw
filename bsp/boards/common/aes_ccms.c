/**
\brief AES CCMS implementation
  
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#include <string.h>
#include <stdint.h>
#include "opendefs.h"
#include "aes_ccms.h"
#include "crypto_engine.h"

/**
\brief CCM* forward transformation (i.e. encryption + authentication) specific to IEEE 802.15.4E.
\param[in] a Pointer to the authentication only data.
\param[in] len_a Length of authentication only data.
\param[in,out] m Pointer to the data that is both authenticated and encrypted. Overwritten by
   ciphertext and the trailing authentication tag. Buffer must hold len_m + CBC_MAC_SIZE.
\param[in,out] len_m Length of data that is both authenticated and encrypted. Accounts for
   the added authentication tag of CBC_MAC_SIZE octets on return.
\param[in] saddr Buffer containing source address (8 octets). Used to create a nonce.
\param[in] asn Buffer containing the Absolute Slot Number (5 octets). Used to create a nonce.
\param[in] key Buffer containing the secret key (16 octets).

\returns E_SUCCESS when the generation was successful, E_FAIL otherwise. 
*/
owerror_t aes_ccms_enc(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t saddr[8],
         uint8_t asn[5],
         uint8_t key[16]) {

   uint8_t mac[CBC_MAC_SIZE];

   if (CRYPTO_ENGINE.aes_cbc_mac(a, len_a, m, *len_m, saddr, asn, key, mac, CBC_MAC_SIZE) == E_SUCCESS) {
      if (CRYPTO_ENGINE.aes_ctr_enc(m, *len_m, saddr, asn, key, mac, CBC_MAC_SIZE) == E_SUCCESS) {
         memcpy(&m[*len_m], mac, CBC_MAC_SIZE);
         *len_m += CBC_MAC_SIZE;

         return E_SUCCESS;
      }
   }

   return E_FAIL;
}

/**
\brief CCM* inverse transformation (i.e. decryption + tag verification) specific to IEEE 802.15.4E.
\param[in] a Pointer to the authentication only data.
\param[in] len_a Length of authentication only data.
\param[in,out] m Pointer to the data that is both authenticated and encrypted. Overwritten by
   plaintext.
\param[in,out] len_m Length of data that is both authenticated and encrypted, including the
   trailing authentication tag. On return it is reduced for CBC_MAC_SIZE octets to account for the
   removed authentication tag.
\param[in] saddr Buffer containing source address (8 octets). Used to create a nonce.
\param[in] asn Buffer containing the Absolute Slot Number (5 octets). Used to create a nonce.
\param[in] key Buffer containing the secret key (16 octets).

\returns E_SUCCESS when decryption and verification were successful, E_FAIL otherwise. 
*/
owerror_t aes_ccms_dec(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t saddr[8],
         uint8_t asn[5],
         uint8_t key[16]) {

   uint8_t mac[CBC_MAC_SIZE];
   uint8_t orig_mac[CBC_MAC_SIZE];

   *len_m -= CBC_MAC_SIZE;
   memcpy(mac, &m[*len_m], CBC_MAC_SIZE);

   if (CRYPTO_ENGINE.aes_ctr_enc(m, *len_m, saddr, asn, key, mac, CBC_MAC_SIZE) == E_SUCCESS) {
      if (CRYPTO_ENGINE.aes_cbc_mac(a, len_a, m, *len_m, saddr, asn, key, orig_mac, CBC_MAC_SIZE) == E_SUCCESS) {
         if (memcmp(mac, orig_mac, CBC_MAC_SIZE) == 0) {
            return E_SUCCESS;
         }
      }
   }

   return E_FAIL;
}
