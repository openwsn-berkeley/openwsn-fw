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
#include <source/flash.h> 

#include "gpio.h"

//=========================== variables =======================================

#define BSP_BUTTON_PORT                 ( GPIO_C_PORT )
#define BSP_BUTTON_USER                 ( GPIO_PIN_3 )
#define BSP_BUTTON_EDGE                 ( GPIO_FALLING_EDGE )

#define CC2538_FLASH_ADDRESS            ( 0x0027F800 )

//=========================== prototypes ======================================

static void button_handler(void);

//=========================== public ==========================================

/**
 * Configures the user button as input source
 */
void button_init(void) {
  volatile uint32_t i;

  /* The button is an input GPIO on falling edge */
  gpio_config_input(BSP_BUTTON_PORT, BSP_BUTTON_USER, BSP_BUTTON_EDGE);

  /* Delay to avoid pin floating problems */
  for (i = 0xFFFFF; i != 0; i--);

  /* Register the interrupt */
  gpio_register_callback(BSP_BUTTON_PORT, BSP_BUTTON_USER, &button_handler);

  /* Clear and enable the interrupt */
  gpio_enable_interrupt(BSP_BUTTON_PORT, BSP_BUTTON_USER);
}

//=========================== private =========================================

/**
 * GPIO_C, PIN_3 interrupt handler
 * Erases a Flash sector to trigger the bootloader backdoor
 */
static void button_handler(void) {
  /* Disable the interrupts */
  IntMasterDisable();

  /* Eras the CCA flash page */
  FlashMainPageErase(CC2538_FLASH_ADDRESS);

  /* Reset the board */
  SysCtrlReset();
}

//=========================== interrupt handlers ==============================
