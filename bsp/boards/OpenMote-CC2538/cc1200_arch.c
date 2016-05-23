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

#define CC1200_SPI_CS_PORT                ( GPIO_D_PORT )
#define CC1200_SPI_CS_PIN                 ( GPIO_PIN_0 )

#define CC1200_GPIO0_PORT                 ( GPIO_C_PORT )
#define CC1200_GPIO0_PIN                  ( GPIO_PIN_3 )
#define CC1200_GPIO0_EDGE                 ( GPIO_FALLING_EDGE )

#define CC1200_GPIO2_PORT                 ( GPIO_C_PORT )
#define CC1200_GPIO2_PIN                  ( GPIO_PIN_3 )
#define CC1200_GPIO2_EDGE                 ( GPIO_FALLING_EDGE )

#define CC1200_GPIO3_PORT                 ( GPIO_C_PORT )
#define CC1200_GPIO3_PIN                  ( GPIO_PIN_3 )
#define CC1200_GPIO3_EDGE                 ( GPIO_FALLING_EDGE )

//=========================== variables =======================================


//=========================== prototypes ======================================

static void cc1200_arch_gpio0_interrupt(void);
static void cc1200_arch_gpio2_interrupt(void);
static void cc1200_arch_gpio3_interrupt(void);

//=========================== public ==========================================

void cc1200_arch_init(void) {
  /* Configure the SPI chip select pin */
  gpio_config_output(CC1200_SPI_CS_PORT, CC1200_SPI_CS_PIN);
  gpio_off(CC1200_SPI_CS_PORT, CC1200_SPI_CS_PIN);

  /* Configure the interrupt */
  gpio_config_input(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN, CC1200_GPIO0_EDGE);
  gpio_config_input(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN, CC1200_GPIO2_EDGE);
  gpio_config_input(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN, CC1200_GPIO3_EDGE);

  /* Register the interrupt */
  gpio_register_callback(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN, &cc1200_arch_gpio0_interrupt);
  gpio_register_callback(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN, &cc1200_arch_gpio2_interrupt);
  gpio_register_callback(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN, &cc1200_arch_gpio3_interrupt);

  /* Clear and enable the interrupt */
  gpio_enable_interrupt(CC1200_GPIO0_PORT, CC1200_GPIO0_PIN);
  gpio_enable_interrupt(CC1200_GPIO2_PORT, CC1200_GPIO2_PIN);
  gpio_enable_interrupt(CC1200_GPIO3_PORT, CC1200_GPIO3_PIN);
}

void cc1200_arch_spi_select(void) {
  gpio_on(CC1200_SPI_CS_PORT, CC1200_SPI_CS_PIN);
}

void cc1200_arch_spi_deselect(void) {
  gpio_off(CC1200_SPI_CS_PORT, CC1200_SPI_CS_PIN);
}

//=========================== private =========================================


//=========================== callbacks =======================================


//=========================== interrupt handlers ==============================

static void cc1200_arch_gpio0_interrupt(void) {
  cc1200_gpio0_interrupt();
}

static void cc1200_arch_gpio2_interrupt(void) {
  cc1200_gpio2_interrupt();
}

static void cc1200_arch_gpio3_interrupt(void) {
  cc1200_gpio3_interrupt();
}