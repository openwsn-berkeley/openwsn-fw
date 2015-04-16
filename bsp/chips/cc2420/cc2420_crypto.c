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

static owerror_t cc2420_launch_enc_and_auth(uint8_t len_a,
                                 uint8_t len_mac,
                                 uint8_t key_index,
                                 cc2420_status_t *status);

static void create_cc2420_nonce(uint8_t l, 
                           uint8_t len_mac,
                           uint8_t len_a,
                           uint8_t* nonce154,
                           uint8_t* buffer);

static void reverse(uint8_t* in, uint8_t len);

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
      do {
         radio_spiStrobe(CC2420_SNOP, &status);
      }
      while (status.enc_busy == 1);
      
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
   uint8_t key_index;
   uint8_t total_message_len;
   uint8_t buffer[128];
   uint8_t cc2420_nonce[16];
   cc2420_status_t status;

   if (!((len_mac == 0) || (len_mac == 4) || (len_mac == 8) || (len_mac == 16))) {
      return E_FAIL;
   }

   // CRC needs to be accounted for in the message len but does not affect encryption
   total_message_len = len_a + *len_m + len_mac + LENGTH_CRC;

   if (total_message_len > 127) { // CC2420 FIFO size is the limiting factor
      return E_FAIL;
   }

   // load key
   if (cc2420_crypto_load_key(key, &key_index) == E_SUCCESS) {
         
      // make sure the Additional Data is concatenated with plaintext
      // add the length byte at the beginning that CC2420 expects
      memcpy(buffer, &total_message_len, 1);
      memcpy(&buffer[1], a, len_a);
      memcpy(&buffer[len_a + 1], m, *len_m);
      memset(&buffer[len_a + *len_m + 1], 0x00, len_mac + LENGTH_CRC); // CC2420 expects MIC and CRC bytes allocated 

      // Write the message to the TX FIFO for encryption and/or authentication
      radio_spiWriteRam(CC2420_RAM_TXFIFO_ADDR, &status, buffer, 16, total_message_len + 1);

      // Create and write the nonce to the CC2420 RAM
      create_cc2420_nonce(l, len_mac, len_a, nonce, cc2420_nonce);
      radio_spiWriteRam(CC2420_RAM_TXNONCE_ADDR, &status, cc2420_nonce, 16);
      
      // need to separate the calls to mimic CC2420 operation
      // both encryption and authentication
      if (len_mac > 0 && *len_m > 0) {
         cc2420_launch_enc_and_auth(len_a, len_mac, key_index, &status);
      }
      // authentication only
      else if (len_mac > 0 && *len_m == 0) {
         return E_FAIL;
      }
      //encryption only (insecure)
      else if (len_mac == 0 && *len_m > 0) {
         return E_FAIL;
      }
   
      // Once command is launched, busy wait for the crypt block to finish
      do {
         radio_spiStrobe(CC2420_SNOP, &status);
      }
      while (status.enc_busy == 1);

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

   uint8_t reversed[16];

   // verify if crystal oscillator is stable
   radio_spiStrobe(CC2420_SNOP, &status);

   if (status.xosc16m_stable) {
      memcpy(reversed, key, CC2420_KEY_LEN);
      reverse(reversed, CC2420_KEY_LEN);

      // Load the key in key RAM
      radio_spiWriteRam(CC2420_RAM_KEY0_ADDR, &status, reversed, CC2420_KEY_LEN);

      *key_index = CC2420_SECCTRL0_KEY_SEL_KEY0;
      return E_SUCCESS;
   }
   return E_FAIL;
}

// private function launching both authentication + encryption 
static owerror_t cc2420_launch_enc_and_auth(uint8_t len_a,
                                 uint8_t len_mac,
                                 uint8_t key_index,
                                 cc2420_status_t *status) {
   cc2420_SECCTRL0_reg_t cc2420_SECCTRL0_reg;
   cc2420_SECCTRL1_reg_t cc2420_SECCTRL1_reg;

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


   // Write to two CC2420 security registers
   radio_spiWriteReg(CC2420_SECCTRL0_ADDR, 
                     status,
                     *(uint16_t*)&cc2420_SECCTRL0_reg);
               
   radio_spiWriteReg(CC2420_SECCTRL1_ADDR, 
                     status,
                     *(uint16_t*)&cc2420_SECCTRL1_reg);

   // issue STXENC to encrypt but not start the transmission
   radio_spiStrobe(CC2420_STXENC, status);
   return E_SUCCESS;
}

static void create_cc2420_nonce(uint8_t l, 
                           uint8_t len_mac,
                           uint8_t len_a,
                           uint8_t* nonce154,
                           uint8_t* buffer) {
   // Create nonce
   buffer[0] = 0x00; // set flags to zero including reserved
   buffer[0] |= 0x07 & (l-1); // field L
   // (len_mac - 2)/2 shifted left 3 times corresponds to (len_mac - 2) << 2
   buffer[0] |= len_mac == 0 ? 0 : (0x07 & (len_mac - 2)) << 2; // field M
   buffer[0] |= len_a != 0 ? 0x40 : 0; // field Adata
   memcpy(&buffer[1], nonce154, 13);
   if (l == 3) { buffer[13] = 0; }
   buffer[14] = 0x00;
   buffer[15] = 0x00; // should this be zero or one?
}

static void reverse(uint8_t *start, uint8_t len) {
   uint8_t *lo = start;
   uint8_t *hi = start + len - 1;
   uint8_t swap;
   while (lo < hi) {
      swap = *lo;
      *lo++ = *hi;
      *hi-- = swap;
   }
}

