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

//=========================== prototypes ======================================
static owerror_t cc2420_crypto_load_key(uint8_t key[16], uint8_t* /* out */ key_index);

static owerror_t cc2420_crypto_inline_ccms_enc(uint8_t* a,
                                 uint8_t len_a,
                                 uint8_t* m,
                                 uint8_t* len_m,
                                 uint8_t* nonce,
                                 uint8_t l,
                                 uint8_t key[16],
                                 uint8_t len_mac);

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
   return E_FAIL;
}

owerror_t cc2420_crypto_ccms_enc(uint8_t* a,
                        uint8_t len_a,
                        uint8_t* m,
                        uint8_t* len_m,
                        uint8_t* nonce,
                        uint8_t l,
                        uint8_t key[16],
                        uint8_t len_mac) {
   // need to separate the calls to mimic CC2420 operation
   // both encryption and authentication
   if (len_mac > 0 && *len_m > 0) {
      return cc2420_crypto_inline_ccms_enc(a,
                                          len_a,
                                          m,
                                          len_m,
                                          nonce,
                                          l,
                                          key,
                                          len_mac);
   }
   // authentication only
   else if (len_mac > 0 && *len_m == 0) {
      return E_FAIL;
   }
   //encryption only (insecure)
   else if (len_mac == 0 && *len_m > 0) {
      return E_FAIL;
   }
   return E_FAIL;
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
   *key_index = CC2420_SECCTRL0_KEY_SEL_KEY0;
   return E_SUCCESS;
}

// private function supporting only authentication + encryption 
static owerror_t cc2420_crypto_inline_ccms_enc(uint8_t* a,
                                 uint8_t len_a,
                                 uint8_t* m,
                                 uint8_t* len_m,
                                 uint8_t* nonce,
                                 uint8_t l,
                                 uint8_t key[16],
                                 uint8_t len_mac) {
   uint8_t key_index;
   uint8_t buffer[128];
   uint8_t cc2420_nonce[16];
   uint8_t total_message_len;
   cc2420_SECCTRL0_reg_t cc2420_SECCTRL0_reg;
   cc2420_SECCTRL1_reg_t cc2420_SECCTRL1_reg;
   cc2420_status_t status;


   if (!((len_mac == 4) || (len_mac == 8) || (len_mac == 16))) {
      return E_FAIL;
   }

   if (*len_m > 125 || *len_m == 0) {
      return E_FAIL;
   }

   // CRC needs to be accounted for in the message len but does not affect encryption
   total_message_len = len_a + *len_m + len_mac + LENGTH_CRC;

   if (total_message_len > 127) {
      return E_FAIL;
   }


   // load key
   if (cc2420_crypto_load_key(key, &key_index) == E_SUCCESS) {
               // make sure the Additional Data is concatenated with plaintext
               memcpy(buffer, a, len_a);
               memcpy(&buffer[len_a], m, *len_m);
               memcpy(&buffer[len_a + *len_m], 0x00, len_mac ); // CC2420 expects MIC and CRC bytes allocated 

               // load in tx buffer the concatenated message in order to use inline encryption mode
               radio_spiWriteTxFifo(&status, buffer, total_message_len);


               //configure SECCTRL0 and SECTRL1
               // key selection for TX
               // write nonce for TX
               // SECCTRL0.SEC_MODE = CCM

               cc2420_SECCTRL0_reg.SEC_MODE = CC2420_SECCTRL0_SEC_MODE_CCM;
               cc2420_SECCTRL0_reg.SEC_M = (len_mac - 2) >> 1; // (M-2)/2
               cc2420_SECCTRL0_reg.SEC_RXKEYSEL = 0;
               cc2420_SECCTRL0_reg.SEC_TXKEYSEL = key_index;
               cc2420_SECCTRL0_reg.SEC_SAKEYSEL = 0;
               cc2420_SECCTRL0_reg.SEC_CBC_HEAD = 1; // use len_a as first byte of authenticated data
               cc2420_SECCTRL0_reg.RXFIFO_PROTECTION = 0;
               cc2420_SECCTRL0_reg.reserved_w0 = 0;

               /// Security Control Register 1
               cc2420_SECCTRL1_reg.SEC_RXL = 0;
               cc2420_SECCTRL1_reg.reserved_1_w0 = 0;
               cc2420_SECCTRL1_reg.SEC_TXL = len_a; // number of bytes until first encrypted byte
               cc2420_SECCTRL1_reg.reserved_2_w0 = 0;

               cc2420_nonce[0] = 0x00; // set flags to zero including reserved
               cc2420_nonce[0] |= 0x07 & (l-1); // field L
               // (len_mac - 2)/2 shifted left 3 times corresponds to (len_mac - 2) << 2
               cc2420_nonce[0] |= len_mac == 0 ? 0 : (0x07 & (len_mac - 2)) << 2; // field M
               cc2420_nonce[0] |= len_a != 0 ? 0x40 : 0; // field Adata
   
               memcpy(&cc2420_nonce[1], nonce, 13);

               if (l == 3) {
                  cc2420_nonce[13] = 0;
               }

               cc2420_nonce[14] = 0x00;
               cc2420_nonce[15] = 0x00; // should this be zero or one?
               
               // Write the nonce to the CC2420 RAM
               radio_spiWriteRam(CC2420_RAM_TXNONCE_ADDR, &status, cc2420_nonce, 16);

               // Write to two CC2420 security registers
               radio_spiWriteReg(CC2420_SECCTRL0_ADDR, 
                     &status,
                     *(uint16_t*)&cc2420_SECCTRL0_reg);
               
               radio_spiWriteReg(CC2420_SECCTRL1_ADDR, 
                     &status,
                     *(uint16_t*)&cc2420_SECCTRL1_reg);

               // issue STXENC to encrypt but not start the transmission
               radio_spiStrobe(CC2420_STXENC, &status);
               while (status.enc_busy == 1) {
                  radio_spiStrobe(CC2420_SNOP, &status);
               }
 
               // FOR DEBUGGING: read the ciphertext from TX fifo with ReadRam()
               radio_spiReadRam(CC2420_RAM_TXFIFO_ADDR,
                         &status,
                         buffer,
                         total_message_len + 1); // plus one for the length byte

               if (buffer[0] != total_message_len) {
                  return E_FAIL;
               }

               // Write ciphertext to vector m[]
               radio_spiReadRam(CC2420_RAM_TXFIFO_ADDR + 1 + len_a, // one for the length byte
                         &status,
                         m,
                         *len_m + len_mac); // ciphertext plus MIC

               *len_m += len_mac;

               // flush TX Fifo ???
               radio_spiStrobe(CC2420_SFLUSHTX, &status);
               radio_spiStrobe(CC2420_SFLUSHTX, &status);

               return E_SUCCESS;
   }
   return E_FAIL;
}


