/**
 * Author: Adam Sedmak (adam.sedmak@gmail.com)
 * Company: Faculty of Electronics and Computing, Zagreb, Croatia
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "spi" bsp module.
 */

#include "sdk/components/boards/pca10056.h"
#include "sdk/components/boards/boards.h"
#include "sdk/components/libraries/delay/nrf_delay.h"
#include "sdk/integration/nrfx/legacy/nrf_drv_spi.h"
#include "sdk/components/libraries/util/app_error.h"

#include <string.h>
#include "stdint.h"
#include "spi.h"
#include "board.h"
#include "board_info.h"

//=========================== defines =========================================
#define SPI_SS_PIN 29
#define SPI_MISO_PIN 28
#define SPI_MOSI_PIN 4
#define SPI_SCK_PIN 3

#define SPI_INSTANCE  0 /**< SPI instance index. */
#define BUF_SIZE 20

//=========================== variables =======================================
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */
static uint8_t m_rx_buf[BUF_SIZE];
//=========================== prototypes ======================================
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context);
//=========================== public ==========================================
void spi_init(void)
{
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;

    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));
	
}
 
void    spi_txrx(uint8_t*     bufTx,
                 uint8_t      lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint8_t      maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast)
{

}

// interrupt handlers
//kick_scheduler_t spi_isr(void)
//{

//}
//=========================== private =========================================

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
    //NRF_LOG_INFO("Transfer completed.");
    if (m_rx_buf[0] != 0)
    {
        //NRF_LOG_INFO(" Received:");
        //NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));
    }
}
