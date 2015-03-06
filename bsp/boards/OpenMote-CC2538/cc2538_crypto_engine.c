/**
\brief Crypto engine implementation for OpenMote-CC2538
  
\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/
#include <stdint.h>
#include "cc2538_crypto_engine.h"
#include "aes_ccms.h"
#include "aes_ctr.h"
#include "aes_cbc.h"
#include "aes_ecb.h"

static owerror_t init(void) {
   return E_SUCCESS;
}
/*---------------------------------------------------------------------------*/
const struct crypto_engine cc2538_crypto_engine = {
   aes_ccms_enc,
   aes_ccms_dec,
   aes_cbc_mac,
   aes_cbc_enc_raw,
   aes_ctr_enc,
   aes_ctr_enc_raw,
   aes_ecb_enc,
   init,
};
/*---------------------------------------------------------------------------*/

