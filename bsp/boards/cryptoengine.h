/**
\brief Definitions for crypto engine initialization

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
\author Timothy Claeys <timothy.claeys@gmail.com>, October 2018.
*/
#ifndef __CRYPTO_ENGINE_H__
#define __CRYPTO_ENGINE_H__

#include "opendefs.h"
#include "source/pka.h"

//=========================== define ==========================================

#define CBC_MAX_MAC_SIZE  16
#define ECC_MAX_MUL_SIZE  24


/* Elliptic Curve groups supported */

enum EC_GROUPS {
    SECP256R1    = 1,
};

//=========================== module variables ================================

/* [NIST P-256, X9.62 prime256v1] */
static const char curve_name[11] = "NIST P-256";
static const uint32_t nist_p_256_p[8] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
                                          0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF };
static const uint32_t nist_p_256_n[8] = { 0xFC632551, 0xF3B9CAC2, 0xA7179E84, 0xBCE6FAAD,
                                          0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF };
static const uint32_t nist_p_256_a[8] = { 0xFFFFFFFC, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
                                          0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF };
static const uint32_t nist_p_256_b[8] = { 0x27D2604B, 0x3BCE3C3E, 0xCC53B0F6, 0x651D06B0,
                                          0x769886BC, 0xB3EBBD55, 0xAA3A93E7, 0x5AC635D8 };
static const uint32_t nist_p_256_x[8] = { 0xD898C296, 0xF4A13945, 0x2DEB33A0, 0x77037D81,
                                          0x63A440F2, 0xF8BCE6E5, 0xE12C4247, 0x6B17D1F2 };
static const uint32_t nist_p_256_y[8] = { 0x37BF51F5, 0xCBB64068, 0x6B315ECE, 0x2BCE3357,
                                          0x7C0F9E16, 0x8EE7EB4A, 0xFE1A7F9B, 0x4FE342E2 };

//============================ module methods =================================

/**
\brief CCM* forward transformation (i.e. encryption + authentication).
\param[in] a Pointer to the authentication only data.
\param[in] len_a Length of authentication only data.
\param[in,out] m Pointer to the data that is both authenticated and encrypted. Overwritten by
    ciphertext and the trailing authentication tag. Buffer must hold len_m + len_mac.
\param[in,out] len_m Length of data that is both authenticated and encrypted. Accounts for
    the added authentication tag of len_mac octets on return.
\param[in] nonce Buffer containing nonce (max 13 octets).
\param[in] l CCM parameter L that allows selection of different nonce length which is (15 - L).
    For example, l = 2 selects 13-octet long nonce which is used for IEEE 802.15.4 security. 
\param[in] key Buffer containing the secret key (16 octets).
\param[in] len_mac Length of the authentication tag.
*/
owerror_t cryptoengine_aes_ccms_enc(uint8_t* a,
      uint8_t len_a,
      uint8_t* m,
      uint8_t* len_m,
      uint8_t* nonce,
      uint8_t l,
      uint8_t key[16],
      uint8_t len_mac);

/**
\brief CCM* inverse transformation (i.e. decryption + tag verification).
\param[in] a Pointer to the authentication only data.
\param[in] len_a Length of authentication only data.
\param[in,out] m Pointer to the data that is both authenticated and encrypted. Overwritten by
    plaintext.
\param[in,out] len_m Length of data that is both authenticated and encrypted, including the
    trailing authentication tag. On return it is reduced for len_mac octets to account for the
    removed authentication tag.
\param[in] nonce Buffer containing nonce (max 13 octets).
\param[in] l CCM parameter L that allows selection of different nonce length which is (15 - L).
 For example, l = 2 selects 13-octet long nonce which is used for IEEE 802.15.4 security.
\param[in] key Buffer containing the secret key (16 octets).
\param[in] len_mac Length of the authentication tag.
*/
owerror_t cryptoengine_aes_ccms_dec(uint8_t* a,
      uint8_t len_a,
      uint8_t* m,
      uint8_t* len_m,
      uint8_t* nonce,
      uint8_t l,
      uint8_t key[16],
      uint8_t len_mac);  



/**
\brief ECDSA signature verification routine.
\param[in] grp Pointer to a struct containing information about the used eliptic curve group.
\param[in] Q Pointer to the public key used for verification of the signature.
\param[in] s Pointer to the s value of the signature.
\param[in] s_length number of uint32_t blocks used for the s value.
\param[in] r Pointer to the r value of the signature.
\param[in] s_length number of uint32_t blocks used for the r value.
\param[in] hash the hash of the message that was signed.
\param[in] h_length number of uint32_t blocks used for the hash.
 */
owerror_t cryptoengine_ecdsa_verify(
      tECCCurveInfo* grp,
      tECPt* Q,
      uint32_t* s,
      uint8_t s_length,
      uint32_t* r,
      uint8_t r_length,
      uint32_t* hash,
      uint8_t h_lenght);


/**
\brief Loads the information on the requested group
\param[in] group Integer denoting the requested group
\param[in, out] grp Pointer to a struct of type tECCCurveInfo,
    struct gets initialized with the correct parameters.
*/
owerror_t cryptoengine_load_group(uint8_t group, tECCCurveInfo *grp);
/**
\brief Basic AES encryption of a single 16-octet block.
\param[in,out] buffer Single block plaintext (16 octets). Will be overwritten by ciphertext.
\param[in] key Buffer containing the secret key (16 octets).
 */
owerror_t cryptoengine_aes_ecb_enc(uint8_t buffer[16], uint8_t key[16]);
    
/**
\brief Initialization of the cryptoengine module.
*/
owerror_t cryptoengine_init(void);

#endif /* __CRYPTO_ENGINE_H__ */
