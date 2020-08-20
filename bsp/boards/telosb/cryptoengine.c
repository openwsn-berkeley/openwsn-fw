/**
\brief CC2420-specific implementation of AES encryption.

\author Malisa Vucinic <malishav@gmail.com>, April 2015.
*/

#include "config.h"

#if BOARD_CRYPTOENGINE_ENABLED

#include "opendefs.h"
#include "board.h"
#include "radio.h"
#include "cc2420.h"
#include "spi.h"
#include "debugpins.h"
#include "cryptoengine.h"


//=========================== defines =========================================

#define CC2420_KEY_LEN           16

#define CC2420_FLAG_MIC_SUCCESS       0x00
#define CC2420_FLAG_MIC_FAIL          0xFF

//=========================== SECCTRL0 values =================================
// SEC_MODE[1:0]
#define CC2420_SECCTRL0_SEC_MODE_DISABLE     0
#define CC2420_SECCTRL0_SEC_MODE_CBC_MAC     1
#define CC2420_SECCTRL0_SEC_MODE_CTR         2
#define CC2420_SECCTRL0_SEC_MODE_CCM         3


// type of operation
#define CC2420_SEC_STANDALONE    CC2420_SECCTRL0_SEC_MODE_DISABLE
#define CC2420_SEC_CBC_MAC       CC2420_SECCTRL0_SEC_MODE_CBC_MAC
#define CC2420_SEC_CTR           CC2420_SECCTRL0_SEC_MODE_CTR
#define CC2420_SEC_CCM           CC2420_SECCTRL0_SEC_MODE_CCM

// encryption/decryption
#define CC2420_SEC_SA_ENC        0
#define CC2420_SEC_ENC           1
#define CC2420_SEC_DEC           2


//=========================== prototypes ======================================
static owerror_t cc2420_crypto_load_key(uint8_t key[16], uint8_t* /* out */ key_index);

static owerror_t cc2420_conf_sec_regs(uint8_t mode,
                                 uint8_t enc_flag,
                                 uint8_t offset,
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
owerror_t cryptoengine_init(void) {
   radio_rfOn();  // turn the crystal oscillator on in order to access CC2420 RAM
   return E_SUCCESS;
}


owerror_t cryptoengine_aes_ecb_enc(uint8_t* buffer, uint8_t* key) {
   cc2420_status_t status;
   uint8_t key_index;

   if (cc2420_crypto_load_key(key, &key_index) == E_SUCCESS) {

      cc2420_conf_sec_regs(CC2420_SEC_STANDALONE, CC2420_SEC_SA_ENC, 0, 0, key_index, &status);

      // write plaintext to the stand-alone buffer
      cc2420_spiWriteRam(CC2420_RAM_SABUF_ADDR, &status, buffer, 16);
      
      // launch stand-alone AES encryption
      cc2420_spiStrobe(CC2420_SAES, &status);
      do {
         cc2420_spiStrobe(CC2420_SNOP, &status);
      }
      while (status.enc_busy == 1);
      
      // read the ciphertext and overwrite the original buffer
      cc2420_spiReadRam(CC2420_RAM_SABUF_ADDR, &status, buffer, 16);
      return E_SUCCESS;
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
   uint8_t key_index;
   uint8_t total_message_len;
   uint8_t buffer[128];
   uint8_t cc2420_nonce[16];
   cc2420_status_t status;

   if (!((len_mac == 0) || (len_mac == 4) || (len_mac == 8) || (len_mac == 16))) {
      return E_FAIL;
   }

   // CRC needs to be accounted for in the message len but does not affect encryption
   total_message_len = len_a + *len_m + LENGTH_CRC;

   if (total_message_len > 127) { // CC2420 FIFO size is the limiting factor
      return E_FAIL;
   }

   // load key
   if (cc2420_crypto_load_key(key, &key_index) == E_SUCCESS) {
         
      // make sure the Associated Data is concatenated with plaintext
      memcpy(buffer, a, len_a);
      memcpy(&buffer[len_a], m, *len_m);

      // Create and write the nonce to the CC2420 RAM
      create_cc2420_nonce(l, len_mac, len_a, nonce, cc2420_nonce);
      cc2420_spiWriteRam(CC2420_RAM_RXNONCE_ADDR, &status, cc2420_nonce, 16);
     
      // It seems that MODE_CCM of CC2420 can do authentication only but not
      // decryption only. For the decryption only case, we then use directly MODE_CTR.
      cc2420_conf_sec_regs(len_mac != 0 ? CC2420_SECCTRL0_SEC_MODE_CCM : CC2420_SECCTRL0_SEC_MODE_CTR,
                           CC2420_SEC_DEC, len_a, len_mac, key_index, &status);

      // Now is the time to transfer the message to FIFO
      cc2420_spiStrobe(CC2420_SFLUSHRX, &status);
      cc2420_spiStrobe(CC2420_SFLUSHRX, &status);
      // To launch decryption in RX FIFO, we must write to it over its register, not direct RAM access
      cc2420_spiWriteFifo(&status, buffer, total_message_len, CC2420_RXFIFO_ADDR);

      // issue STXENC to encrypt but not start the transmission
      cc2420_spiStrobe(CC2420_SRXDEC, &status);

      // Once command is launched, busy wait for the crypt block to finish
      do {
         cc2420_spiStrobe(CC2420_SNOP, &status);
      } while (status.enc_busy == 1);

      cc2420_spiReadRam(CC2420_RAM_RXFIFO_ADDR + 1 + len_a, // one for the length byte
                         &status,
                         buffer,
                         *len_m); // length that includes MIC

      // verify MIC flag replaced by CC2420 in the last byte of the message
      if (len_mac == 0 || buffer[*len_m - 1] == CC2420_FLAG_MIC_SUCCESS) {
         *len_m -= len_mac;
         memcpy(m, buffer, *len_m);

         // clean up
         cc2420_spiStrobe(CC2420_SFLUSHRX, &status);
         cc2420_spiStrobe(CC2420_SFLUSHRX, &status);
         return E_SUCCESS;
      }
   }
   return E_FAIL;
}


owerror_t cryptoengine_aes_ccms_enc(uint8_t* a,
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
         
      // make sure the Associated Data is concatenated with plaintext
      memcpy(buffer, a, len_a);
      memcpy(&buffer[len_a], m, *len_m);

      // Create and write the nonce to the CC2420 RAM
      create_cc2420_nonce(l, len_mac, len_a, nonce, cc2420_nonce);
      cc2420_spiWriteRam(CC2420_RAM_TXNONCE_ADDR, &status, cc2420_nonce, 16);
 
      // It seems that MODE_CCM of CC2420 can do authentication only but not
      // encryption only. For the encryption only case, we then use directly MODE_CTR.
      cc2420_conf_sec_regs(len_mac != 0 ? CC2420_SECCTRL0_SEC_MODE_CCM : CC2420_SECCTRL0_SEC_MODE_CTR,
                           CC2420_SEC_ENC, len_a, len_mac, key_index, &status);
 
      // Now is the time to transfer the message to FIFO
      cc2420_spiStrobe(CC2420_SFLUSHTX, &status);
      // Message must be transfered to TX FIFO using its special purpose register.
      cc2420_spiWriteFifo(&status, buffer, total_message_len, CC2420_TXFIFO_ADDR);
    
      // issue STXENC to encrypt but not start the transmission
      cc2420_spiStrobe(CC2420_STXENC, &status);

      // Once command is launched, busy wait for the crypt block to finish
      do {
         cc2420_spiStrobe(CC2420_SNOP, &status);
      } while (status.enc_busy == 1);

      // Write ciphertext to vector m[]
      cc2420_spiReadRam(CC2420_RAM_TXFIFO_ADDR + 1 + len_a, // one for the length byte
                         &status,
                         m,
                         *len_m + len_mac); // ciphertext plus MIC

      *len_m += len_mac;

      // flush TX Fifo ???
      cc2420_spiStrobe(CC2420_SFLUSHTX, &status);
      return E_SUCCESS;
   }
   return E_FAIL;
}

//=========================== private =========================================

/**
\brief On success, returns by reference the location in key RAM where the 
   new/existing key is stored. Minimizes SPI transfer at the cost of RAM usage.
*/
static owerror_t cc2420_crypto_load_key(uint8_t key[16], uint8_t* /* out */ key_index) {
   static uint8_t loaded_key[2][16];   // to save some SPI transfers, keep a copy of
                                       // loaded keys in MCU RAM.
   static uint8_t num_keys_loaded = 0;
   cc2420_status_t status;
   uint8_t reversed[16];
   uint8_t i;
   uint8_t next_key_index;

   // verify if crystal oscillator is stable
   cc2420_spiStrobe(CC2420_SNOP, &status);

   if (status.xosc16m_stable) {  // green light only if CC2420 is ready

      for (i = 0; i < 2; i++) {
         if (num_keys_loaded > i && memcmp(loaded_key[i], key, 16) == 0) { // first cond. to avoid all zero key hit
            *key_index = i;
            return E_SUCCESS;
         }
      }

      next_key_index = num_keys_loaded & 0x01;

      memcpy(loaded_key[next_key_index], key, CC2420_KEY_LEN);
      memcpy(reversed, loaded_key[next_key_index], CC2420_KEY_LEN);
      // Need to transfer it little-endian
      reverse(reversed, CC2420_KEY_LEN);

       // Load the key in key RAM
      cc2420_spiWriteRam(next_key_index == 0 ? CC2420_RAM_KEY0_ADDR : CC2420_RAM_KEY1_ADDR,
                        &status, reversed, CC2420_KEY_LEN);

      *key_index = next_key_index;
      num_keys_loaded++;
      return E_SUCCESS;
   }
   return E_FAIL;
}

// private function that configures registers for different encryption modes 
static owerror_t cc2420_conf_sec_regs(uint8_t mode,
                                 uint8_t enc_flag,
                                 uint8_t offset,
                                 uint8_t len_mac,
                                 uint8_t key_index,
                                 cc2420_status_t *status) {
   cc2420_SECCTRL0_reg_t cc2420_SECCTRL0_reg;
   cc2420_SECCTRL1_reg_t cc2420_SECCTRL1_reg;

   memset(&cc2420_SECCTRL0_reg, 0x00, sizeof(cc2420_SECCTRL0_reg_t));
   memset(&cc2420_SECCTRL1_reg, 0x00, sizeof(cc2420_SECCTRL1_reg_t));

   //configure SECCTRL0 and SECTRL1
   cc2420_SECCTRL0_reg.SEC_MODE = mode;
   cc2420_SECCTRL0_reg.SEC_M = len_mac != 0 ? (len_mac - 2) >> 1 : 0; // (M-2)/2 or 0

   switch (enc_flag) {
      case CC2420_SEC_ENC:
         cc2420_SECCTRL0_reg.SEC_TXKEYSEL = key_index;
         cc2420_SECCTRL1_reg.SEC_TXL = offset; 
         break;
      case CC2420_SEC_DEC:
         cc2420_SECCTRL0_reg.SEC_RXKEYSEL = key_index;
         cc2420_SECCTRL1_reg.SEC_RXL = offset;
         break;
      case CC2420_SEC_SA_ENC:
         cc2420_SECCTRL0_reg.SEC_SAKEYSEL = key_index;
         break;
      default:
         return E_FAIL;
   }

   cc2420_SECCTRL0_reg.SEC_CBC_HEAD = 1; // use offset as first byte of authenticated data
   cc2420_SECCTRL0_reg.RXFIFO_PROTECTION = 0;
   cc2420_SECCTRL0_reg.reserved_w0 = 0;

   cc2420_SECCTRL1_reg.reserved_1_w0 = 0;
   cc2420_SECCTRL1_reg.reserved_2_w0 = 0;

   // Write to two CC2420 security registers
   cc2420_spiWriteReg(CC2420_SECCTRL0_ADDR, 
                     status,
                     *(uint16_t*)&cc2420_SECCTRL0_reg);
               
   cc2420_spiWriteReg(CC2420_SECCTRL1_ADDR, 
                     status,
                     *(uint16_t*)&cc2420_SECCTRL1_reg);

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
   buffer[0] |= len_a != 0 ? 0x08 : 0; // field Adata in CC2420's nonce (see datasheet)
   memcpy(&buffer[1], nonce154, 13); 
   if (l == 3) { buffer[13] = 0; }
   buffer[14] = 0x00; // block counter
   buffer[15] = 0x01; // block counter
   reverse(buffer, 16);
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

#endif /* BOARD_CRYPTOENGINE_ENABLED */
