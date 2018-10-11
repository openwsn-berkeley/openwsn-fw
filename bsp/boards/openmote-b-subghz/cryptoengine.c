/**
\brief Crypto engine implementation for OpenMote-CC2538
  
\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/
#include <stdint.h>

#include <headers/hw_sys_ctrl.h>

#include <source/sys_ctrl.h>
#include <source/aes.h>
#include <source/ccm.h>

#include "cryptoengine.h"

#define DEFAULT_KEY_AREA KEY_AREA_0


//=========================== prototypes ======================================

static owerror_t load_key(uint8_t key[16], uint8_t* /* out */ key_location);


//=========================== public ==========================================

owerror_t cryptoengine_init(void) {
   //
   // Enable AES peripheral
   //
   SysCtrlPeripheralReset(SYS_CTRL_PERIPH_AES);
   SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_AES);
   return E_SUCCESS;
}

owerror_t cryptoengine_aes_ccms_enc(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {

   bool encrypt;
   uint8_t key_location;
  
   encrypt = *len_m > 0 ? true : false;

   if(load_key(key, &key_location) == E_SUCCESS) {
      if(CCMAuthEncryptStart(encrypt,
                              len_mac,
                              nonce,
                              m,
                              (uint16_t) *len_m,
                              a,
                              (uint16_t) len_a,
                              key_location,
                              &m[*len_m],
                              l,
                              /* polling */ 0) == AES_SUCCESS) {

         do {
            ASM_NOP;
         } while(CCMAuthEncryptCheckResult() == 0);
        
         if(CCMAuthEncryptGetResult(len_mac, 
                                    (uint16_t) *len_m,
                                    &m[*len_m]) == AES_SUCCESS) {

            *len_m += len_mac;
            return E_SUCCESS;
         }
      }
   }

   return E_FAIL;
}

owerror_t cryptoengine_aes_ccms_dec(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {

   bool decrypt;
   uint8_t key_location;
   uint8_t tag[CBC_MAX_MAC_SIZE];
  
   decrypt = *len_m - len_mac > 0 ? true : false;

   if(load_key(key, &key_location) == E_SUCCESS) {
      if(CCMInvAuthDecryptStart(decrypt,
                              len_mac,
                              nonce,
                              m,
                              (uint16_t) *len_m,
                              a,
                              (uint16_t) len_a,
                              key_location,
                              tag,
                              l,
                              /* polling */ 0) == AES_SUCCESS) {

         do {
            ASM_NOP;
         } while(CCMInvAuthDecryptCheckResult() == 0);
       
         if(CCMInvAuthDecryptGetResult(len_mac, 
                                       m,
                                       (uint16_t) *len_m,
                                       tag) == AES_SUCCESS) {

            *len_m -= len_mac;
            return E_SUCCESS;
         }
      }
   }
   return E_FAIL;
}

owerror_t cryptoengine_aes_ecb_enc(uint8_t* buffer, uint8_t* key) {
   uint8_t key_location;
   if(load_key(key, &key_location) == E_SUCCESS) {
      // Polling
      if(AESECBStart(buffer, buffer, key_location, 1, 0) == AES_SUCCESS) {
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

//=========================== private ==========================================

/**
\brief On success, returns by reference the location in key RAM where the 
   new/existing key is stored.
*/
static owerror_t load_key(uint8_t key[16], uint8_t* /* out */ key_location) {
   static uint8_t loaded_key[16];
   
   if(memcmp(loaded_key, key, 16) != 0) {
      memcpy(loaded_key, key, 16);
      // Load the key in key RAM
      if(AESLoadKey(loaded_key, DEFAULT_KEY_AREA) != AES_SUCCESS) {
         return E_FAIL;
      }
   }
   *key_location = DEFAULT_KEY_AREA;
   return E_SUCCESS;
}
