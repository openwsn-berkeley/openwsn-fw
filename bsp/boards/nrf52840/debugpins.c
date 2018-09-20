 /**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "debugpins" bsp module.
 */

#include "sdk/modules/nrfx/hal/nrf_gpio.h"

#include "debugpins.h"
#include "stdint.h"
#include "board.h"
#include "board_info.h"


#ifndef DEBUG_PINS_DISABLED


//=========================== defines =========================================

// board debug PINS defines

#if BOARD_PCA10056           // nrf52840-DK
#define GPIO_DEBUGPIN_FRAME  26
#define GPIO_DEBUGPIN_SLOT   27
#define GPIO_DEBUGPIN_FSM    29
#define GPIO_DEBUGPIN_TASK   28
#define GPIO_DEBUGPIN_ISR    30
#define GPIO_DEBUGPIN_RADIO  31
#endif
#if BOARD_PCA10059           // nrf52840-DONGLE
#define GPIO_DEBUGPIN_FRAME  9
#define GPIO_DEBUGPIN_SLOT   10
#define GPIO_DEBUGPIN_FSM    42
#define GPIO_DEBUGPIN_TASK   31
#define GPIO_DEBUGPIN_ISR    22
#define GPIO_DEBUGPIN_RADIO  29
#endif

// the below defines are used to cycle through all pins
#define GPIO_NUM_DEBUGPINS 6
#define GPIO_PINS_LIST { GPIO_DEBUGPIN_SLOT, GPIO_DEBUGPIN_FRAME, GPIO_DEBUGPIN_ISR, GPIO_DEBUGPIN_TASK, GPIO_DEBUGPIN_FSM, GPIO_DEBUGPIN_RADIO }

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void)
{
  // set the corresponding GPIO pins to be used as output
  const uint8_t m_board_gpio_debugpin_list[GPIO_NUM_DEBUGPINS] = GPIO_PINS_LIST;

  for (uint8_t p=0; p < GPIO_NUM_DEBUGPINS; ++p)
  {
    nrf_gpio_cfg_output(m_board_gpio_debugpin_list[p]);
  }
}


void debugpins_frame_set(void)
{
  nrf_gpio_pin_set(GPIO_DEBUGPIN_FRAME);
}

void debugpins_frame_clr(void)
{
  nrf_gpio_pin_clear(GPIO_DEBUGPIN_FRAME);
}

void debugpins_frame_toggle(void)
{
  nrf_gpio_pin_toggle(GPIO_DEBUGPIN_FRAME);
}


void debugpins_slot_set(void)
{
  nrf_gpio_pin_set(GPIO_DEBUGPIN_SLOT);
}

void debugpins_slot_clr(void)
{
  nrf_gpio_pin_clear(GPIO_DEBUGPIN_SLOT);
}

void debugpins_slot_toggle(void)
{
  nrf_gpio_pin_toggle(GPIO_DEBUGPIN_SLOT);
}


void debugpins_fsm_set(void)
{
  nrf_gpio_pin_set(GPIO_DEBUGPIN_FSM);
}

void debugpins_fsm_clr(void)
{
  nrf_gpio_pin_clear(GPIO_DEBUGPIN_FSM);
}

void debugpins_fsm_toggle(void)
{
  nrf_gpio_pin_toggle(GPIO_DEBUGPIN_FSM);
}


void debugpins_task_set(void)
{
  nrf_gpio_pin_set(GPIO_DEBUGPIN_TASK);
}

void debugpins_task_clr(void)
{
  nrf_gpio_pin_clear(GPIO_DEBUGPIN_TASK);
}

void debugpins_task_toggle(void)
{
  nrf_gpio_pin_toggle(GPIO_DEBUGPIN_TASK);
}


void debugpins_isr_set(void)
{
  nrf_gpio_pin_set(GPIO_DEBUGPIN_ISR);
}

void debugpins_isr_clr(void)
{
  nrf_gpio_pin_clear(GPIO_DEBUGPIN_ISR);
}

void debugpins_isr_toggle(void)
{
  nrf_gpio_pin_toggle(GPIO_DEBUGPIN_ISR);
}


void debugpins_radio_set(void)
{
  nrf_gpio_pin_set(GPIO_DEBUGPIN_RADIO);
}

void debugpins_radio_clr(void)
{
  nrf_gpio_pin_clear(GPIO_DEBUGPIN_RADIO);
}

void debugpins_radio_toggle(void)
{
  nrf_gpio_pin_toggle(GPIO_DEBUGPIN_RADIO);
}


//------------ private ------------//

#endif // DEBUG_PINS_DISABLED