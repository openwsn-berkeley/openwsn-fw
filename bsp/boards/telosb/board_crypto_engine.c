/**
\brief Crypto engine implementation for TelosB (CC2420)
  
\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/
#include <stdint.h>
#include "board_crypto_engine.h"
#include "aes_ecb.h"
#include "aes_ctr.h"
#include "aes_cbc.h"
#include "aes_ccms.h"
#include "cc2420_crypto.h"
#include "radio.h"

static owerror_t init(void) {
   radio_rfOn();  // turn the crystal oscillator on in order to access CC2420 RAM
   return E_SUCCESS;
}

/*---------------------------------------------------------------------------*/
const struct crypto_engine board_crypto_engine = {
   cc2420_crypto_ccms_enc,
   cc2420_crypto_ccms_dec,
   aes_cbc_enc_raw,
   aes_ctr_enc_raw,
   cc2420_crypto_aes_ecb_enc,      // AES stand-alone encryption
   init,
};
/*---------------------------------------------------------------------------*/

