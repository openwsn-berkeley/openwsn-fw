/**
\brief nRF5340_network-specific implementation of AES encryption.

\author: Tengfei Chang <tengfei.chang@inria.fr> August 2020
*/

#include "opendefs.h"
#include "board.h"
#include "radio.h"
#include "debugpins.h"
#include "cryptoengine.h"

//=========================== defines =========================================

//=========================== prototypes ======================================

//=========================== public ==========================================
owerror_t cryptoengine_init(void) {
   // todo
}

owerror_t cryptoengine_aes_ecb_enc(uint8_t* buffer, uint8_t* key) {
    // todo
}

owerror_t cryptoengine_aes_ccms_dec(uint8_t* a,
                        uint8_t len_a,
                        uint8_t* m,
                        uint8_t* len_m,
                        uint8_t* nonce,
                        uint8_t l,
                        uint8_t key[16],
                        uint8_t len_mac) {
   
    // todo
}

owerror_t cryptoengine_aes_ccms_enc(uint8_t* a,
                        uint8_t len_a,
                        uint8_t* m,
                        uint8_t* len_m,
                        uint8_t* nonce,
                        uint8_t l,
                        uint8_t key[16],
                        uint8_t len_mac) {
   
    // todo
}

//=========================== private =========================================

