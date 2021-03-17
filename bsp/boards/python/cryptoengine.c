/**
\brief Wrapper of software implementation of CCM.

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/

#include "cryptoengine.h"

owerror_t cryptoengine_aes_ccms_enc(uint8_t *a,
                                    uint8_t len_a,
                                    uint8_t *m,
                                    uint8_t *len_m,
                                    uint8_t *nonce,
                                    uint8_t l,
                                    uint8_t key[16],
                                    uint8_t len_mac) {
    (void) a;
    (void) len_a;
    (void) len_m;
    (void) m;
    (void) nonce;
    (void) l;
    (void) key;
    (void) len_mac;

    return E_FAIL;

}

owerror_t cryptoengine_aes_ccms_dec(uint8_t *a,
                                    uint8_t len_a,
                                    uint8_t *m,
                                    uint8_t *len_m,
                                    uint8_t *nonce,
                                    uint8_t l,
                                    uint8_t key[16],
                                    uint8_t len_mac) {
    (void) a;
    (void) len_a;
    (void) len_m;
    (void) m;
    (void) nonce;
    (void) l;
    (void) key;
    (void) len_mac;

    return E_FAIL;
}

owerror_t cryptoengine_aes_ecb_enc(uint8_t *buffer, uint8_t *key) {
    (void) buffer;
    (void) key;

    return E_FAIL;
}

owerror_t cryptoengine_init(void) {
    return E_FAIL;
}

