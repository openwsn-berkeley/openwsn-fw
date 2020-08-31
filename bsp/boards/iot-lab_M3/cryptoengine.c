/**
\brief Implementation of cryptoengine based on AT86RF231's hardware accelerator.
*/

#include "config.h"

#if BOARD_CRYPTOENGINE_ENABLED

#include <stdint.h>
#include <string.h>
#include "at86rf231.h"
#include "cryptoengine.h"
#include "spi.h"
#include "radio.h"

//=========================== prototypes ======================================
owerror_t at86rf231_crypto_load_key(uint8_t key[16]);
owerror_t at86rf231_crypto_opt_ecb(uint8_t* buffer);
static owerror_t aes_cbc_mac(uint8_t* a, uint8_t len_a, uint8_t* m, uint8_t len_m, uint8_t* nonce, uint8_t* mac, uint8_t len_mac, uint8_t l);
static owerror_t aes_ctr_enc(uint8_t* m, uint8_t len_m, uint8_t* nonce, uint8_t* mac, uint8_t len_mac, uint8_t l);
owerror_t aes_cbc_enc_raw(uint8_t* buffer, uint8_t len, uint8_t iv[16]);
owerror_t aes_ctr_enc_raw(uint8_t* buffer, uint8_t len, uint8_t iv[16]);
static void inc_counter(uint8_t* counter);

//=========================== public ==========================================

owerror_t cryptoengine_aes_ccms_enc(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   uint8_t mac[CBC_MAX_MAC_SIZE];

   if ((len_mac > CBC_MAX_MAC_SIZE) || (l != 2)) {
      return E_FAIL;
   }

   if (at86rf231_crypto_load_key(key) != E_SUCCESS) {
        return E_FAIL;
   }

   if (aes_cbc_mac(a, len_a, m, *len_m, nonce, mac, len_mac, l) == E_SUCCESS) {
      if (aes_ctr_enc(m, *len_m, nonce, mac, len_mac, l) == E_SUCCESS) {
         memcpy(&m[*len_m], mac, len_mac);
         *len_m += len_mac;

         return E_SUCCESS;
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
   uint8_t mac[CBC_MAX_MAC_SIZE];
   uint8_t orig_mac[CBC_MAX_MAC_SIZE];

   if ((len_mac > CBC_MAX_MAC_SIZE) || (l != 2)) {
      return E_FAIL;
   }

   if (at86rf231_crypto_load_key(key) != E_SUCCESS) {
        return E_FAIL;
   }

   *len_m -= len_mac;
   memcpy(mac, &m[*len_m], len_mac);

   if (aes_ctr_enc(m, *len_m, nonce, mac, len_mac, l) == E_SUCCESS) {
      if (aes_cbc_mac(a, len_a, m, *len_m, nonce, orig_mac, len_mac, l) == E_SUCCESS) {
         if (memcmp(mac, orig_mac, len_mac) == 0) {
            return E_SUCCESS;
         }
      }
   }

   return E_FAIL;
}

owerror_t cryptoengine_aes_ecb_enc(uint8_t* buffer, uint8_t* key) {
    if (at86rf231_crypto_load_key(key) == E_SUCCESS) {
        at86rf231_crypto_opt_ecb(buffer);
        return E_SUCCESS;
    }
    return E_FAIL;
}

owerror_t cryptoengine_init(void) {
    radio_rfOn();
    return E_SUCCESS;
}

//=========================== private =========================================
// Optimized ECB AES encryption that does not load the key beforehand
owerror_t at86rf231_crypto_opt_ecb(uint8_t* buffer) {
    uint8_t spi_tx_buffer[3];
    uint8_t spi_rx_buffer[16];

    uint8_t aes_cmd;
    uint8_t aes_status;

    aes_cmd = 0x80; // AES start
    aes_status = 0x00;

    spi_tx_buffer[0] = 0x40; // SRAM write
    spi_tx_buffer[1] = RG_AES_CTRL; // AES_CTRL register
    spi_tx_buffer[2] = 0x00; // ECB encryption

    spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_NOTLAST);

    spi_txrx((uint8_t*)buffer,
            16,
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_NOTFIRST,
            SPI_NOTLAST);

    spi_txrx(&aes_cmd,
            sizeof(aes_cmd),
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_NOTFIRST,
            SPI_LAST);

    // Prepare to read the AES status register
    spi_tx_buffer[0] = 0x00;
    spi_tx_buffer[1] = RG_AES_STATUS;

    // Busy wait reading AES status register until it is done or an error occurs
    do {
        spi_txrx(spi_tx_buffer,
                sizeof(spi_tx_buffer),
                SPI_BUFFER,
                (uint8_t*)spi_rx_buffer,
                sizeof(spi_rx_buffer),
                SPI_FIRST,
                SPI_LAST);
        aes_status = spi_rx_buffer[2];
    } while((aes_status & 0x01) == 0x00);

    if ((aes_status & 0x80) == 0x01) {
        // an error occured
        return E_FAIL;
    }

    spi_tx_buffer[0] = 0x00;
    spi_tx_buffer[1] = RG_AES_STATE_KEY;

    // send the command to read the ciphertext
    spi_txrx(spi_tx_buffer,
                2,
                SPI_BUFFER,
                (uint8_t*)spi_rx_buffer,
                16,
                SPI_FIRST,
                SPI_NOTLAST);

    // read the actual ciphertext
    spi_txrx(spi_tx_buffer,
                16,
                SPI_BUFFER,
                (uint8_t*)buffer,
                16,
                SPI_NOTFIRST,
                SPI_LAST);

    return E_SUCCESS;
}


owerror_t at86rf231_crypto_load_key(uint8_t key[16]) {
    uint8_t spi_tx_buffer[3];
    uint8_t spi_rx_buffer[16];

    spi_tx_buffer[0] = 0x40; // SRAM write
    spi_tx_buffer[1] = RG_AES_CTRL; // AES_CTRL register
    spi_tx_buffer[2] = 0x10; // key mode

    spi_txrx(spi_tx_buffer,
            sizeof(spi_tx_buffer),
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_FIRST,
            SPI_NOTLAST);

    spi_txrx((uint8_t*)key,
            16,
            SPI_BUFFER,
            (uint8_t*)spi_rx_buffer,
            sizeof(spi_rx_buffer),
            SPI_NOTFIRST,
            SPI_LAST);

    return E_SUCCESS;
}

/**
\brief Raw AES-CBC encryption omitting everything but the last block. Assumes key
    is already loaded in the engine.
\param[in,out] buffer Message to be encrypted. First and last block will be overwritten.
\param[in] len Message length. Must be multiple of 16 octets.
\param[in] iv Buffer containing the Initialization Vector (16 octets).

\returns E_SUCCESS when the encryption was successful.
*/
owerror_t aes_cbc_enc_raw(uint8_t* buffer, uint8_t len, uint8_t iv[16]) {
    uint8_t  n;
    uint8_t  k;
    uint8_t  nb;
    uint8_t* pbuf;
    uint8_t* pxor;
    uint8_t spi_tx_buffer[3];
    uint8_t spi_rx_buffer[16];
    uint8_t aes_cmd;
    uint8_t aes_status;

    nb = len >> 4;
    pxor = iv;
    pbuf = buffer;
    for (k = 0; k < 16; k++) {
        pbuf[k] ^= pxor[k];
    }
    at86rf231_crypto_opt_ecb(pbuf);

    aes_cmd = 0xA0; // AES-CBC encryption start
    aes_status = 0x00;
    // first block already done, start actual CBC processing in hardware
    for (n = 1; n<nb; n++) {
        pbuf = &buffer[16 * n];
        spi_tx_buffer[0] = 0x40; // SRAM write
        spi_tx_buffer[1] = RG_AES_CTRL; // AES_CTRL register
        spi_tx_buffer[2] = 0x20; // CBC encryption

        spi_txrx(spi_tx_buffer,
                sizeof(spi_tx_buffer),
                SPI_BUFFER,
                (uint8_t*)spi_rx_buffer,
                sizeof(spi_rx_buffer),
                SPI_FIRST,
                SPI_NOTLAST);

        spi_txrx((uint8_t*)pbuf,
                16,
                SPI_BUFFER,
                (uint8_t*)spi_rx_buffer,
                sizeof(spi_rx_buffer),
                SPI_NOTFIRST,
                SPI_NOTLAST);

        spi_txrx(&aes_cmd,
                sizeof(aes_cmd),
                SPI_BUFFER,
                (uint8_t*)spi_rx_buffer,
                sizeof(spi_rx_buffer),
                SPI_NOTFIRST,
                SPI_LAST);

        // Prepare to read the AES status register
        spi_tx_buffer[0] = 0x00;
        spi_tx_buffer[1] = RG_AES_STATUS;

        // Busy wait reading AES status register until it is done or an error occurs
        do {
            spi_txrx(spi_tx_buffer,
                    sizeof(spi_tx_buffer),
                    SPI_BUFFER,
                    (uint8_t*)spi_rx_buffer,
                    sizeof(spi_rx_buffer),
                    SPI_FIRST,
                    SPI_LAST);
            aes_status = spi_rx_buffer[2];
        } while((aes_status & 0x01) == 0x00);

        if ((aes_status & 0x80) == 0x01) {
            // an error occured
            return E_FAIL;
        }
    }

    spi_tx_buffer[0] = 0x00;
    spi_tx_buffer[1] = RG_AES_STATE_KEY;

    // send the command to read the last block
    spi_txrx(spi_tx_buffer,
                2,
                SPI_BUFFER,
                (uint8_t*)spi_rx_buffer,
                16,
                SPI_FIRST,
                SPI_NOTLAST);

    // read the actual ciphertext
    spi_txrx(spi_tx_buffer,
                16,
                SPI_BUFFER,
                (uint8_t*)pbuf,
                16,
                SPI_NOTFIRST,
                SPI_LAST);

   return E_SUCCESS;
}

/**
\brief CBC-MAC generation specific to CCM*. Assumes key is already loaded in the
engine.
\param[in] a Pointer to the authentication only data.
\param[in] len_a Length of authentication only data.
\param[in] m Pointer to the data that is both authenticated and encrypted.
\param[in] len_m Length of data that is both authenticated and encrypted.
\param[in] nonce Buffer containing nonce (13 octets).
\param[out] mac Buffer where the value of the CBC-MAC tag will be written.
\param[in] len_mac Length of the CBC-MAC tag. Must be 4, 8 or 16 octets.
\param[in] l CCM parameter L that allows selection of different nonce length.

\returns E_SUCCESS when the generation was successful, E_FAIL otherwise.
*/
static owerror_t aes_cbc_mac(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t len_m,
         uint8_t* nonce,
         uint8_t* mac,
         uint8_t len_mac,
         uint8_t l) {

   uint8_t  pad_len;
   uint8_t  len;
   uint8_t  cbc_mac_iv[16];
   uint8_t  buffer[128+16+16]; // max buffer plus IV

   // asserts here
   if (!((len_mac == 0) || (len_mac == 4) || (len_mac == 8) || (len_mac == 16))) {
      return E_FAIL;
   }

   if ((len_a > 127) || (len_m > 127) || ((len_a + len_m) > 127)) {
      return E_FAIL;
   }

   if (mac == 0) {
      return E_FAIL;
   }

   memset(cbc_mac_iv, 0x00, 16); // CBC-MAC Initialization Vector is a zero string

   // IV: flags (1B) | SADDR (8B) | ASN (5B) | len(m) (2B)
   // X0 xor IV in first 16 bytes of buffer: set buffer[:16] as IV)
   buffer[0] = 0x00; // set flags to zero including reserved
   buffer[0] |= 0x07 & (l-1); // field L
   // (len_mac - 2)/2 shifted left es corresponds to (len_mac - 2) << 2
   buffer[0] |= len_mac == 0 ? 0 : (0x07 & (len_mac - 2)) << 2; // field M
   buffer[0] |= len_a != 0 ? 0x40 : 0; // field Adata

   memcpy(&buffer[1], nonce, 13);

   if (l == 3) {
      buffer[13] = 0;
   }

   buffer[14] = 0;
   buffer[15] = len_m;

   len = 16;
   // len(a)
   if(len_a > 0) {
      buffer[16] = 0;
      buffer[17] = len_a;
      len += 2;
   }

   //  (((x >> 4) + 1)<<4) - x   or    16 - (x % 16) ?
   // a + padding
   pad_len = ((((len_a + len - 16) >> 4) + 1) << 4) - (len_a + len - 16);
   pad_len = pad_len == 16 ? 0 : pad_len;
   memcpy(&buffer[len], a, len_a);
   len += len_a;
   memset(&buffer[len], 0, pad_len);
   len += pad_len;

   // m + padding
   pad_len = (((len_m >> 4) + 1) << 4) - len_m;
   pad_len = pad_len == 16 ? 0 : pad_len;
   memcpy(&buffer[len], m, len_m);
   len += len_m;
   memset(&buffer[len], 0, pad_len);
   len += pad_len;

   aes_cbc_enc_raw(buffer, len, cbc_mac_iv);

   // copy MAC
   memcpy(mac, &buffer[len - 16], len_mac);

   return E_SUCCESS;
}

/**
\brief Counter (CTR) mode encryption specific to IEEE 802.15.4E. Assumes key is already
loaded in the engine.
\param[in,out] m Pointer to the data that is both authenticated and encrypted. Data is
   overwritten by ciphertext (i.e. plaintext in case of inverse CCM*).
\param[in] len_m Length of data that is both authenticated and encrypted.
\param[in] nonce Buffer containing nonce (13 octets).
\param[in,out] mac Buffer containing the unencrypted or encrypted CBC-MAC tag, which depends
   on weather the function is called as part of CCM* forward or inverse transformation. It
   is overwrriten by the encrypted, i.e unencrypted, tag on return.
\param[in] len_mac Length of the CBC-MAC tag. Must be 4, 8 or 16 octets.
\param[in] l CCM parameter L that allows selection of different nonce length.

\returns E_SUCCESS when the encryption was successful, E_FAIL otherwise.
*/
static owerror_t aes_ctr_enc(uint8_t* m,
         uint8_t len_m,
         uint8_t* nonce,
         uint8_t* mac,
         uint8_t len_mac,
         uint8_t l) {

   uint8_t pad_len;
   uint8_t len;
   uint8_t iv[16];
   uint8_t buffer[128 + 16]; // max buffer plus mac

   // asserts here
   if (!((len_mac == 0) || (len_mac == 4) || (len_mac == 8) || (len_mac == 16))) {
      return E_FAIL;
   }

   if (len_m > 127) {
      return E_FAIL;
   }

   // iv (flag (1B) | source addr (8B) | ASN (5B) | cnt (2B)
   iv[0] = 0x00;
   iv[0] |= 0x07 & (l-1); // field L

   memcpy(&iv[1], nonce, 13);
   iv[14] = 0x00;
   iv[15] = 0x00;

   // first block is mac
   memcpy(buffer, mac, len_mac);
   memset(&buffer[len_mac], 0, 16 - len_mac);
   len = 16;

   //  (((x >> 4) + 1)<<4) - x   or    16 - (x % 16) ?
   // m + padding
   pad_len = (((len_m >> 4) + 1) << 4) - len_m;
   pad_len = pad_len == 16 ? 0 : pad_len;
   memcpy(&buffer[len], m, len_m);
   len += len_m;
   memset(&buffer[len], 0, pad_len);
   len += pad_len;

   aes_ctr_enc_raw(buffer, len, iv);

   memcpy(m, &buffer[16], len_m);
   memcpy(mac, buffer, len_mac);

   return E_SUCCESS;
}

/**
\brief Raw AES-CTR encryption. Assumes key is already loaded in the engine.
\param[in,out] buffer Message to be encrypted. Will be overwritten by ciphertext.
\param[in] len Message length. Must be multiple of 16 octets.
\param[in] iv Buffer containing the Initialization Vector (16 octets).

\returns E_SUCCESS when the encryption was successful.
*/
owerror_t aes_ctr_enc_raw(uint8_t* buffer, uint8_t len, uint8_t iv[16]) {
   uint8_t n;
   uint8_t k;
   uint8_t nb;
   uint8_t* pbuf;
   uint8_t eiv[16];

   nb = len >> 4;
   for (n = 0; n < nb; n++) {
      pbuf = &buffer[16 * n];
      memcpy(eiv, iv, 16);
      at86rf231_crypto_opt_ecb(eiv);
      // may be faster if vector are aligned to 4 bytes (use long instead char in xor)
      for (k = 0; k < 16; k++) {
         pbuf[k] ^= eiv[k];
      }
      inc_counter(iv);
   }

   return E_SUCCESS;
}

static void inc_counter(uint8_t* counter) {
   // from openssl
   uint32_t n = 16;
   uint8_t  c;
   do {
      --n;
      c = counter[n];
      ++c;
      counter[n] = c;
      if (c) return;
   } while (n);
}

#endif /* BOARD_CRYPTOENGINE_ENABLED */
