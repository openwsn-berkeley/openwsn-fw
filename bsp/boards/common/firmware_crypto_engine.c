/**
\brief Crypto engine initialization
  
\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/
#include <stdint.h>
#include <string.h>
#include "opendefs.h"
#include "firmware_crypto_engine.h"
#include "aes_ccms.h"
#include "aes_ctr.h"
#include "aes_cbc.h"
#include "aes_ecb.h"

static owerror_t init(void) {
   return E_SUCCESS;
}

/*---------------------------------------------------------------------------*/
const struct crypto_engine firmware_crypto_engine = {
   aes_ccms_enc,
   aes_ccms_dec,
   aes_cbc_enc_raw,
   aes_ctr_enc_raw,
   aes_ecb_enc,
   init,
};
/*---------------------------------------------------------------------------*/

