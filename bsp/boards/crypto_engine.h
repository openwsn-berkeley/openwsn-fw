/**
\brief Definitions for crypto engine initialization

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
*/
#ifndef __CRYPTO_ENGINE_H__
#define __CRYPTO_ENGINE_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "opendefs.h"

//=========================== define ==========================================
#define CBC_MAX_MAC_SIZE  16

#ifdef CRYPTO_ENGINE_SCONS
#define CRYPTO_ENGINE CRYPTO_ENGINE_SCONS
#else /* CRYPTO_ENGINE_SCONS */
#define CRYPTO_ENGINE dummy_crypto_engine
#endif /* CRYPTO_ENGINE_SCONS */

//=========================== module variables ================================

struct crypto_engine {

   /**
   \brief CCM* forward transformation (i.e. encryption + authentication) specific to IEEE 802.15.4E.
   \param[in] a Pointer to the authentication only data.
   \param[in] len_a Length of authentication only data.
   \param[in,out] m Pointer to the data that is both authenticated and encrypted. Overwritten by
      ciphertext and the trailing authentication tag. Buffer must hold len_m + CBC_MAC_SIZE.
   \param[in,out] len_m Length of data that is both authenticated and encrypted. Accounts for
      the added authentication tag of CBC_MAC_SIZE octets on return.
   \param[in] nonce Buffer containing nonce (13 octets).
   \param[in] key Buffer containing the secret key (16 octets).
   \param[in] len_mac Length of the authentication tag.
   */
   owerror_t (* aes_ccms_enc)(uint8_t* a,
      uint8_t len_a,
      uint8_t* m,
      uint8_t* len_m,
      uint8_t nonce[13],
      uint8_t key[16],
      uint8_t len_mac);

   /**
   \brief CCM* inverse transformation (i.e. decryption + tag verification) specific to IEEE 802.15.4E.
   \param[in] a Pointer to the authentication only data.
   \param[in] len_a Length of authentication only data.
   \param[in,out] m Pointer to the data that is both authenticated and encrypted. Overwritten by
      plaintext.
   \param[in,out] len_m Length of data that is both authenticated and encrypted, including the
      trailing authentication tag. On return it is reduced for CBC_MAC_SIZE octets to account for the
      removed authentication tag.
   \param[in] nonce Buffer containing nonce (13 octets).
   \param[in] key Buffer containing the secret key (16 octets).
   \param[in] len_mac Length of the authentication tag.
   */
   owerror_t (* aes_ccms_dec)(uint8_t* a,
      uint8_t len_a,
      uint8_t* m,
      uint8_t* len_m,
      uint8_t nonce[13],
      uint8_t key[16],
      uint8_t len_mac);  

   /**
   \brief CBC-MAC generation specific to IEEE 802.15.4E.
   \param[in] a Pointer to the authentication only data.
   \param[in] len_a Length of authentication only data.
   \param[in] m Pointer to the data that is both authenticated and encrypted.
   \param[in] len_m Length of data that is both authenticated and encrypted.
   \param[in] nonce Buffer containing nonce (13 octets).
   \param[in] key Buffer containing the secret key (16 octets).
   \param[out] mac Buffer where the value of the CBC-MAC tag will be written.
   \param[in] len_mac Length of the CBC-MAC tag.
   */
   owerror_t (* aes_cbc_mac)(uint8_t* a,
      uint8_t len_a,
      uint8_t* m,
      uint8_t len_m,
      uint8_t nonce[13],
      uint8_t key[16],
      uint8_t* mac,
      uint8_t len_mac);
    
   /**
   \brief Raw AES-CBC encryption.
   \param[in,out] buffer Message to be encrypted. Will be overwritten by ciphertext.
   \param[in] len Message length. 
   \param[in] key Buffer containing the secret key (16 octets).
   */  
   owerror_t (* aes_cbc_enc_raw)(uint8_t* buffer,
      uint8_t len,
      uint8_t key[16]);

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
   \param[in] len_mac Length of the CBC-MAC tag.
   */
   owerror_t (* aes_ctr_enc)(uint8_t* m,
      uint8_t len_m,
      uint8_t nonce[13],
      uint8_t key[16],
      uint8_t* mac,
      uint8_t len_mac);

   /**
   \brief Raw AES-CTR encryption.
   \param[in,out] buffer Message to be encrypted. Will be overwritten by ciphertext.
   \param[in] len Message length.
   \param[in] key Buffer containing the secret key (16 octets).
   \param[in] iv Buffer containing the Initialization Vector (16 octets).
   */
   owerror_t (* aes_ctr_enc_raw)(uint8_t* buffer,
      uint8_t len,
      uint8_t key[16],
      uint8_t iv[16]);

   /**
   \brief Basic AES encryption of a single 16-octet block.
   \param[in,out] buffer Single block plaintext (16 octets). Will be overwritten by ciphertext.
   \param[in] key Buffer containing the secret key (16 octets).
   */
   owerror_t (* aes_ecb_enc)(uint8_t buffer[16],
      uint8_t key[16]);
    
   /**
   \brief Initialization of the crypto_engine driver.
   */
   owerror_t (* init)(void);
}; // struct crypto_engine

extern const struct crypto_engine CRYPTO_ENGINE;

#ifdef  __cplusplus
}
#endif

#endif /* __CRYPTO_ENGINE_H__ */
