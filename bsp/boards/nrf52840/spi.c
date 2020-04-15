/**
 * Author: Adam Sedmak (adam.sedmak@gmail.com)
 * Company: Faculty of Electronics and Computing, Zagreb, Croatia
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "spi" bsp module.
 */

#include "sdk/components/boards/boards.h"
#include "sdk/modules/nrfx/drivers/include/nrfx_spi.h"

#include "app_config.h"
#include "leds.h"
#include "spi.h"
#include "board.h"
#include "board_info.h"
#include "debugpins.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

//=========================== defines =========================================

#if BOARD_PCA10056  // nrf52840-DK
  #define SPI_SS_PIN   33   ///< P1.01
  #define SPI_MISO_PIN 34   ///< P1.02
  #define SPI_MOSI_PIN 35   ///< P1.03
  #define SPI_SCK_PIN  36   ///< P1.04
#endif
#if BOARD_PCA10059  // nrf52840-DONGLE
  #define SPI_SS_PIN   02
  #define SPI_MISO_PIN 15
  #define SPI_MOSI_PIN 17
  #define SPI_SCK_PIN  20
#endif


#define SPI_INSTANCE  0   ///< SPI instance index


//=========================== variables =======================================

static const nrfx_spi_t spi = NRFX_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */


//=========================== prototypes ======================================

void spi_event_handler(nrfx_spi_evt_t const * p_event,void * p_context);


//=========================== public ==========================================

void spi_init(void) {
    nrfx_spi_config_t spi_config = NRFX_SPI_DEFAULT_CONFIG;

    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;

    if(NRF_SUCCESS != nrfx_spi_init(&spi, &spi_config, spi_event_handler, NULL))
    {
      leds_error_blink();
      board_reset();
    }
}

 
void    spi_txrx(uint8_t*     bufTx,
                 uint16_t     lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint16_t     maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast)
{
    // Fill in transfer descriptor
    nrfx_spi_xfer_desc_t nrfx_spi_xfer_desc;

    nrfx_spi_xfer_desc.p_tx_buffer = bufTx;
    nrfx_spi_xfer_desc.tx_length = lenbufTx;
    nrfx_spi_xfer_desc.p_rx_buffer = bufRx;
    nrfx_spi_xfer_desc.rx_length = maxLenBufRx;

    memset(bufRx, 0, maxLenBufRx);

    spi_xfer_done = false;

    if (NRFX_SUCCESS != nrfx_spi_xfer(&spi, &nrfx_spi_xfer_desc, 0)) {
        leds_error_blink();
    }

    while (!spi_xfer_done) {
        board_sleep();
    }
}


// interrupt handlers
kick_scheduler_t spi_isr(void) {
    return DO_NOT_KICK_SCHEDULER;
}


//=========================== private =========================================

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrfx_spi_evt_t const * p_event,              
                       void *                p_context)
{
    spi_xfer_done = true;
}
