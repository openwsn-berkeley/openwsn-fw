/**
 * Author: Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "CC1200" bsp module.
 */

#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>

#include <source/ioc.h>
#include <source/gpio.h>
#include <source/sys_ctrl.h>

#include "gpio.h"

#include "cc1200_arch.h"

//=========================== defines =========================================

#define CC1200_SPI_CS_PORT                ( GPIO_A_PORT )
#define CC1200_SPI_CS_PIN                 ( GPIO_PIN_3 )

#define CC1200_GPIO0_PORT                 ( GPIO_D_PORT )
#define CC1200_GPIO0_PIN                  ( GPIO_PIN_3 )

#define CC1200_GPIO2_PORT                 ( GPIO_D_PORT )
#define CC1200_GPIO2_PIN                  ( GPIO_PIN_1 )

#define CC1200_GPIO3_PORT                 ( GPIO_D_PORT )
#define CC1200_GPIO3_PIN                  ( GPIO_PIN_0 )

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void cc1200_arch_init(void) {
  /* Configure the SPI chip select pin */
  gpio_config_output(CC1200_SPI_CS_PORT, CC1200_SPI_CS_PIN);
  gpio_off(CC1200_SPI_CS_PORT, CC1200_SPI_CS_PIN);
}

void cc1200_arch_gpio0_setup(bool rising) {
  if (rising) {
    gpio_config_input(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN, GPIO_RISING_EDGE);
  } else {
    gpio_config_input(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN, GPIO_FALLING_EDGE);
  }
  gpio_register_callback(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN, &cc1200_arch_gpio0_interrupt);
}

void cc1200_arch_gpio0_enable(void) {
  gpio_enable_interrupt(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN);
}

void cc1200_arch_gpio0_disable(void) {
  gpio_disable_interrupt(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN);
}

bool cc1200_arch_gpio0_read(void) {
  return gpio_read(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN);
}

void cc1200_arch_gpio2_setup(bool rising) {
  if (rising) {
    gpio_config_input(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN, GPIO_RISING_EDGE);
  } else {
    gpio_config_input(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN, GPIO_FALLING_EDGE);
  }
  gpio_register_callback(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN, &cc1200_arch_gpio2_interrupt);
}

void cc1200_arch_gpio2_enable(void) {
  gpio_enable_interrupt(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN);
}

void cc1200_arch_gpio2_disable(void) {
  gpio_disable_interrupt(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN);
}

bool cc1200_arch_gpio2_read(void) {
  return gpio_read(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN);
}

void cc1200_arch_gpio3_setup(bool rising) {
  if (rising) {
    gpio_config_input(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN, GPIO_RISING_EDGE);
  } else {
    gpio_config_input(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN, GPIO_FALLING_EDGE);
  }
  gpio_register_callback(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN, &cc1200_arch_gpio3_interrupt);
}

void cc1200_arch_gpio3_enable(void) {
  gpio_enable_interrupt(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN);
}

void cc1200_arch_gpio3_disable(void) {
  gpio_disable_interrupt(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN);
}

bool cc1200_arch_gpio3_read(void) {
  return gpio_read(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN);
}

void cc1200_arch_spi_select(void) {
  gpio_on(CC1200_SPI_CS_PORT, CC1200_SPI_CS_PIN);
}

void cc1200_arch_spi_deselect(void) {
  gpio_off(CC1200_SPI_CS_PORT, CC1200_SPI_CS_PIN);
}

uint8_t cc1200_arch_spi_rw_byte(uint8_t byte) {

}

void cc1200_arch_spi_rw(uint8_t* read, const uint8_t* write, uint16_t length) {

}

//=========================== private =========================================


//=========================== callbacks =======================================


//=========================== interrupt handlers ==============================

void cc1200_arch_gpio0_interrupt(void) {
  cc1200_gpio0_interrupt();
}

void cc1200_arch_gpio2_interrupt(void) {
  cc1200_gpio2_interrupt();
}

void cc1200_arch_gpio3_interrupt(void) {
  cc1200_gpio3_interrupt();
}
