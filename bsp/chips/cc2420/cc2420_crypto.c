/**
\brief CC2420-specific implementation of AES encryption.

\author Malisa Vucinic <malishav@gmail.com>, April 2015.
*/

#include "opendefs.h"
#include "board.h"
#include "radio.h"
#include "cc2420.h"
#include "cc2420_crypto.h"
#include "spi.h"
#include "debugpins.h"

#define CC2420_KEY_LEN        16

#define CC2420_KEY_INDEX_0    0
#define CC2420_KEY_INDEX_1    1
//=========================== prototypes ======================================
static owerror_t cc2420_crypto_load_key(uint8_t key[16], uint8_t* /* out */ key_index);

//=========================== public ==========================================

owerror_t cc2420_crypto_aes_ecb_enc(uint8_t* buffer, uint8_t* key) {
   cc2420_SECCTRL0_reg_t cc2420_SECCTRL0_reg;
   cc2420_status_t status;
   uint8_t key_index;

   if (cc2420_crypto_load_key(key, &key_index) == E_SUCCESS) {
      memset(&cc2420_SECCTRL0_reg, 0x00, sizeof(cc2420_SECCTRL0_reg_t));
      // configure the SECCTRL register to use the loaded key
      cc2420_SECCTRL0_reg.SEC_SAKEYSEL = key_index;
      radio_spiWriteReg(CC2420_SECCTRL0_ADDR, 
                     &status,
                     *(uint16_t*)&cc2420_SECCTRL0_reg);

      // write plaintext to the stand-alone buffer
      radio_spiWriteRam(CC2420_RAM_SABUF_ADDR, &status, buffer, 16);
      
      // launch stand-alone AES encryption
      radio_spiStrobe(CC2420_SAES, &status);
      while (status.enc_busy == 1) {
         radio_spiStrobe(CC2420_SNOP, &status);
      }
      
      // read the ciphertext and overwrite the original buffer
      radio_spiReadRam(CC2420_RAM_SABUF_ADDR, &status, buffer, 16);
      return E_SUCCESS;
   }
}

//=========================== private =========================================

/**
\brief On success, returns by reference the location in key RAM where the 
   new/existing key is stored.
*/
static owerror_t cc2420_crypto_load_key(uint8_t key[16], uint8_t* /* out */ key_index) {
   cc2420_status_t status;

   // Load the key in key RAM
   radio_spiWriteRam(CC2420_RAM_KEY0_ADDR, &status, key, CC2420_KEY_LEN);
   *key_index = CC2420_KEY_INDEX_0;
   return E_SUCCESS;
}



