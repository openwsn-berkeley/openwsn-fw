/**
 * Author: Adam Sedmak (adam.sedmak@gmail.com)
 * Company: Faculty of Electronics and Computing, Zagreb, Croatia
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "spi" bsp module.
 */

#include "board_info.h"
#include "spi.h"

//=========================== defines =========================================


#define SPI_SS_PIN   NRF_GPIO_PIN_MAP(1,1)   ///< P1.01
#define SPI_MISO_PIN NRF_GPIO_PIN_MAP(1,2)   ///< P1.02
#define SPI_MOSI_PIN NRF_GPIO_PIN_MAP(1,3)   ///< P1.03
#define SPI_SCK_PIN  NRF_GPIO_PIN_MAP(1,4)   ///< P1.04

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void spi_init(void) {
}

 
void    spi_txrx(uint8_t*     bufTx,
                 uint16_t     lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint16_t     maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast)
{
   
}

//=========================== private =========================================