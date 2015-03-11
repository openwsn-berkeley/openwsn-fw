/**
\brief Crypto engine implementation for OpenMote-CC2538
  
\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/
#include <stdint.h>
#include <headers/hw_sys_ctrl.h>
#include "sys_ctrl.h"
#include "cc2538_crypto_engine.h"
#include "aes_ccms.h"
#include "aes_ctr.h"
#include "aes_cbc.h"
#include "aes.h"  // CC2538 specific headers
#include "ccm.h"  // CC2538 specific headers

#define DEFAULT_KEY_AREA KEY_AREA_0

static owerror_t load_key(uint8_t key[16]) {
   static uint8_t loaded_key[16];
   
   if(memcmp(loaded_key, key, 16) != 0) {
      memcpy(loaded_key, key, 16);
      // Load the key in key RAM
      if(AESLoadKey(loaded_key, DEFAULT_KEY_AREA) != AES_SUCCESS) {
         return E_FAIL;
      }
   }
   return E_SUCCESS;
}

static owerror_t init(void) {
   //
   // Enable AES peripheral
   //
   SysCtrlPeripheralReset(SYS_CTRL_PERIPH_AES);
   SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_AES);
   return E_SUCCESS;
}

static owerror_t aes_ecb_enc_cc2538(uint8_t* buffer, uint8_t* key) {
   if(load_key(key) == E_SUCCESS) {
      // Polling
      if(AESECBStart(buffer, buffer, DEFAULT_KEY_AREA, 1, 0) == AES_SUCCESS) {
         do {
            ASM_NOP;
         } while(AESECBCheckResult() == 0);

         if(AESECBGetResult() == AES_SUCCESS) {
            return E_SUCCESS;
         }
      }
   }
   return E_FAIL;
}
/*---------------------------------------------------------------------------*/
const struct crypto_engine board_crypto_engine = {
   aes_ccms_enc,
   aes_ccms_dec,
   aes_cbc_enc_raw,
   aes_ctr_enc_raw,
   aes_ecb_enc_cc2538,      // AES stand-alone encryption
   init,
};
/*---------------------------------------------------------------------------*/

