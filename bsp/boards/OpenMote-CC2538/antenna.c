/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "board" bsp module.
 */

#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>

#include <source/ioc.h>
#include <source/gpio.h>
#include <source/gptimer.h>
#include <source/sys_ctrl.h>
#include <source/interrupt.h>

#include "gpio.h"
#include "antenna.h"

//=========================== variables =======================================

#define BSP_ANTENNA_PORT                ( GPIO_D_PORT )
#define BSP_ANTENNA_INT                 ( GPIO_PIN_5 )
#define BSP_ANTENNA_EXT                 ( GPIO_PIN_4 )

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
 * Configures the antenna using a RF switch
 * INT is the internal antenna (chip) configured through ANT1_SEL (V1)
 * EXT is the external antenna (connector) configured through ANT2_SEL (V2)
 */
void antenna_init(void) {
  /* Configure the ANT1 and ANT2 GPIO as output */
  gpio_config_output(BSP_ANTENNA_PORT, BSP_ANTENNA_INT);
  gpio_config_output(BSP_ANTENNA_PORT, BSP_ANTENNA_EXT);

  /* By default the chip antenna is selected as the default */
  gpio_on(BSP_ANTENNA_PORT, BSP_ANTENNA_INT);
  gpio_off(BSP_ANTENNA_PORT, BSP_ANTENNA_EXT);
}

/**
 * Selects the external (connector) antenna
 */
void antenna_external(void) {
  gpio_on(BSP_ANTENNA_PORT, BSP_ANTENNA_EXT);
  gpio_off(BSP_ANTENNA_PORT, BSP_ANTENNA_INT);
}

/**
 * Selects the internal (chip) antenna
 */
void antenna_internal(void) {
  gpio_off(BSP_ANTENNA_PORT, BSP_ANTENNA_EXT);
  gpio_on(BSP_ANTENNA_PORT, BSP_ANTENNA_INT);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
