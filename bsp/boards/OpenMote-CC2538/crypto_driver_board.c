/**
  \brief Crypto driver initialization for custom board
  
  \author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>
 */
#include <stdint.h>
#include "crypto_driver.h"
#include "aes_ccms.h"
#include "aes_ctr.h"
#include "aes_cbc_mac.h"
#include "aes_ecb.h"
#include "crypto_driver_board.h"

int crypto_driver_board_init(crypto_driver_t *crypto_driver)
{
   crypto_driver->aes_ccms_dec = aes_ccms_dec;
   crypto_driver->aes_ccms_enc = aes_ccms_enc;
   crypto_driver->aes_cbc_mac_enc = aes_cbc_mac_enc;
   crypto_driver->aes_cbc_mac_enc_raw = aes_cbc_mac_enc_raw; //aes_cbc_mac_board_enc_raw; 
   crypto_driver->aes_ctr_enc = aes_ctr_enc;
   crypto_driver->aes_ctr_enc_raw = aes_ctr_enc_raw; // aes_ctr_board_enc_raw;
   crypto_driver->aes_ecb_enc = aes_ecb_enc; //aes_ecb_board_enc

   return 0;
}
