/**
\brief Dummy implementation of cryptoengine.
*/

#include <stdint.h>
#include <string.h>
#include "at86rf231.h"
#include "cryptoengine.h"
#include "spi.h"
#include "radio.h"

//=========================== prototypes ======================================
owerror_t at86rf231_crypto_load_key(uint8_t key[16]);
owerror_t at86rf231_crypto_opt_ecb(uint8_t* buffer);

//=========================== public ==========================================

owerror_t cryptoengine_aes_ccms_enc(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {
   
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
    uint8_t aes_cmd;

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

