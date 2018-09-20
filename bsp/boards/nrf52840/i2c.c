/**
 * brief nrf52840-specific definition of the "i2c" bsp module.
 *
 * Authors: Tamas Harczos (tamas.harczos@imms.de)
 * Company: Institut fuer Mikroelektronik- und Mechatronik-Systeme gemeinnuetzige GmbH (IMMS GmbH)
 * Date:    August 2018
*/


#include "nrfx_twi.h"

#include "i2c.h"
#include "app_config.h"
#include "leds.h"
#include "board.h"
#include "board_info.h"
#include "nrf_gpio.h"


//=========================== defines =========================================


//=========================== variables =======================================

#if BOARD_PCA10056           // nrf52840-DK
#define I2C_SCL_PIN NRF_GPIO_PIN_MAP(1, 5)
#define I2C_SDA_PIN NRF_GPIO_PIN_MAP(1, 6)
#endif

#if BOARD_PCA10059           // nrf52840-DONGLE
#define I2C_SCL_PIN NRF_GPIO_PIN_MAP(1, 0)
#define I2C_SDA_PIN NRF_GPIO_PIN_MAP(0, 24)
#endif

nrf_twi_frequency_t const I2C_BAUDRATE= NRF_TWI_FREQ_400K;  ///< actual rate is 410256 bps
nrfx_twi_t m_twi= NRFX_TWI_INSTANCE(1);                     ///< instance 0 is used by the SPI


//=========================== prototypes ======================================

//=========================== public ==========================================

void i2c_init(void)
{
  nrfx_twi_config_t const i2c_config=
  {
    .scl= I2C_SCL_PIN,                                          ///< SCL pin number
    .sda= I2C_SDA_PIN,                                          ///< SDA pin number
    .frequency= I2C_BAUDRATE,                                   ///< TWI frequency
    .interrupt_priority= NRFX_TWI_DEFAULT_CONFIG_IRQ_PRIORITY,  ///< interrupt priority
    .hold_bus_uninit= false                                     ///< hold pull up state on GPIO pins after uninit
  };

  if (NRFX_SUCCESS != nrfx_twi_init(&m_twi, &i2c_config, NULL, NULL))
  {
    leds_error_blink();
  }

  // After enabling the TWI, it will draw some current. @todo: Maybe we could enable/disable TWI only during
  // transfers. Or maybe we should disable it every time the board goes to sleep and reinitialize it upon usage. 
  nrfx_twi_enable(&m_twi);
}


void i2c_read_byte(uint8_t address, uint8_t* byte)
{
  i2c_read_bytes(address, byte, 1);
}


uint32_t i2c_read_bytes(uint8_t address, uint8_t* buffer, uint32_t length)
{
  nrfx_err_t retVal= nrfx_twi_rx(&m_twi, address, buffer, (size_t) length);

  if (retVal != NRFX_SUCCESS)
  {
    // handle error
  }

  return length;
}


void i2c_write_byte(uint8_t address, uint8_t byte)
{   
  i2c_write_bytes(address, &byte, 1);
}


uint32_t i2c_write_bytes(uint8_t address, uint8_t* buffer, uint32_t length)
{
  nrfx_err_t retVal= nrfx_twi_tx(&m_twi, address, (uint8_t const *) buffer, (size_t) length, false);

  if (retVal != NRFX_SUCCESS)
  {
    // handle error
  }

  return length;
}

//=========================== private =========================================

