/**
\brief CC2420-specific crypto library with hardware acceleration.

\author Malisa Vucinic <malishav@gmail.com>, April 2015.
*/
#ifndef __CC2420_CRYPTO_H__
#define __CC2420_CRYPTO_H__

//=========================== SECCTRL0 values =================================
// SEC_MODE[1:0]
#define CC2420_SECCTRL0_SEC_MODE_DISABLE     0
#define CC2420_SECCTRL0_SEC_MODE_CBC_MAC     1
#define CC2420_SECCTRL0_SEC_MODE_CTR         2
#define CC2420_SECCTRL0_SEC_MODE_CCM         3

//=========================== prototypes ======================================

owerror_t cc2420_crypto_aes_ecb_enc(uint8_t* buffer, uint8_t* key);

owerror_t cc2420_crypto_ccms_enc(uint8_t* a,
                        uint8_t len_a,
                        uint8_t* m,
                        uint8_t* len_m,
                        uint8_t* nonce,
                        uint8_t l,
                        uint8_t key[16],
                        uint8_t len_mac);

owerror_t cc2420_crypto_ccms_dec(uint8_t* a,
                        uint8_t len_a,
                        uint8_t* m,
                        uint8_t* len_m,
                        uint8_t* nonce,
                        uint8_t l,
                        uint8_t key[16],
                        uint8_t len_mac);

#endif /* __CC2420_CRYPTO_H__ */
