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
   the added authentication tag of len_mac octets on return.
\param[in] nonce Buffer containing nonce (13 octets).
\param[in] l CCM parameter L that allows selection of different nonce length. This implementation
   supports l = 2 (i.e. 13 octet long nonce) only.
\param[in] key Buffer containing the secret key (16 octets).
\param[in] len_mac Length of the authentication tag.

\returns E_SUCCESS when the generation was successful, E_FAIL otherwise. 
*/
owerror_t aes_ccms_enc(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {

   uint8_t mac[CBC_MAX_MAC_SIZE];

   if ((len_mac > CBC_MAX_MAC_SIZE) || (l != 2)) {
      return E_FAIL;
   }

   if (CRYPTO_ENGINE.aes_cbc_mac(a, len_a, m, *len_m, nonce, key, mac, len_mac) == E_SUCCESS) {
      if (CRYPTO_ENGINE.aes_ctr_enc(m, *len_m, nonce, key, mac, len_mac) == E_SUCCESS) {
         memcpy(&m[*len_m], mac, len_mac);
         *len_m += len_mac;

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
   trailing authentication tag. On return it is reduced for len_mac octets to account for the
   removed authentication tag.
\param[in] nonce Buffer containing nonce (13 octets).
\param[in] l CCM parameter L that allows selection of different nonce length. This implementation
   supports l = 2 (i.e. 13 octet long nonce) only.
\param[in] key Buffer containing the secret key (16 octets).
\param[in] len_mac Length of the authentication tag.

\returns E_SUCCESS when decryption and verification were successful, E_FAIL otherwise. 
*/
owerror_t aes_ccms_dec(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {

   uint8_t mac[CBC_MAX_MAC_SIZE];
   uint8_t orig_mac[CBC_MAX_MAC_SIZE];

   if ((len_mac > CBC_MAX_MAC_SIZE) || (l != 2)) {
      return E_FAIL;
   }

   *len_m -= len_mac;
   memcpy(mac, &m[*len_m], len_mac);

   if (CRYPTO_ENGINE.aes_ctr_enc(m, *len_m, nonce, key, mac, len_mac) == E_SUCCESS) {
      if (CRYPTO_ENGINE.aes_cbc_mac(a, len_a, m, *len_m, nonce, key, orig_mac, len_mac) == E_SUCCESS) {
         if (memcmp(mac, orig_mac, len_mac) == 0) {
            return E_SUCCESS;
         }
      }
   }

   return E_FAIL;
}
