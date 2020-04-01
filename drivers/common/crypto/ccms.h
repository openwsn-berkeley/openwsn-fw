/**
\brief Definitions for AES CCMS implementation

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
\author Malisa Vucinic <malishav@gmail.com>, June 2017.
*/
#ifndef OPENWSN_CCMS_H
#define OPENWSN_CCMS_H

//=========================== prototypes ======================================

/**
\brief CCM* forward transformation (i.e. encryption + authentication) implemented in software. Invokes software implementation of AES.
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
owerror_t aes128_ccms_enc(uint8_t *a,
                          uint8_t len_a,
                          uint8_t *m,
                          uint8_t *len_m,
                          uint8_t *nonce,
                          uint8_t l,
                          uint8_t key[16],
                          uint8_t len_mac);

/**
\brief CCM* inverse transformation (i.e. decryption + tag verification) implemented in software. Invokes software implementation of AES.
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
owerror_t aes128_ccms_dec(uint8_t *a,
                          uint8_t len_a,
                          uint8_t *m,
                          uint8_t *len_m,
                          uint8_t *nonce,
                          uint8_t l,
                          uint8_t key[16],
                          uint8_t len_mac);

#endif /* OPENWSN_CCMS_H */
