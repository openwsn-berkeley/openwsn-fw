/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "board" bsp module.
 */

#include "sdk/modules/nrfx/hal/nrf_power.h"
#include "sdk/components/libraries/pwr_mgmt/nrf_pwr_mgmt.h"
#include "sdk/integration/nrfx/legacy/nrf_drv_clock.h"
#include "sdk/components/libraries/delay/nrf_delay.h"

#include "sdk/modules/nrfx/mdk/nrf52840.h"
#include "sdk/modules/nrfx/drivers/nrfx_common.h"
#include "sdk/modules/nrfx/drivers/include/nrfx_systick.h"
#include "sdk/components/libraries/util/nrf_assert.h"
#include "sdk/components/toolchain/cmsis/include/core_cm4.h"
#include "sdk/integration/nrfx/nrfx_glue.h"

#include "sdk/components/boards/boards.h"

#include "board.h"
#include "leds.h"
// #include "bsp_timer.h"   ///< OBSOLETE, use sctimer instead
// #include "radiotimer.h"  ///< OBSOLETE, use sctimer instead
#include "sctimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "spi.h"
#include "radio.h"
#include "sensors.h"
#include "i2c.h"


//=========================== variables =======================================

//=========================== prototypes ======================================

static void button_init(void);


//=========================== main ============================================

extern int mote_main(void);

int main(void)
{
  return mote_main();
}


//=========================== public ==========================================

void board_init(void)
{
  // start low-frequency clock (LFCLK)
  nrf_drv_clock_init();
  NRF_CLOCK->EVENTS_LFCLKSTARTED= 0;  ///< part of workaround for 3.1 [20] RTC: Register values are invalid from http://infocenter.nordicsemi.com/pdf/nRF52840_Rev_1_Errata_v1.1.pdf
  nrf_drv_clock_lfclk_request(NULL);
  while (!nrf_drv_clock_lfclk_is_running()) { }
  NRF_RTC0->TASKS_STOP= 0;            ///< part of workaround for 3.1 [20] RTC: Register values are invalid from http://infocenter.nordicsemi.com/pdf/nRF52840_Rev_1_Errata_v1.1.pdf

  nrfx_systick_init();

  leds_init();

  // enable on-board DC-DC converter to reduce overall consumption (this also disables the LDO [low-dropout] regulator)
  // This only works if the required coil and condenser are properly connected to the pins DCC and DEC4, which is the
  // case with the development kit, but not with some other nRF52840-based boards. (If enabled without the proper 
  // circuitry, the CPU will hang.)
  nrf_power_dcdcen_set(true);
  nrf_power_dcdcen_vddh_set(true);

  // initialize power management library
  nrf_pwr_mgmt_init();

  // initialize board with LEDs and buttons
  bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);
  button_init();

  uart_init();

  debugpins_init();

  sctimer_init();  ///<  bsp_timer_init() and radiotimer_init() were OBSOLETE, we use sctimer instead

  radio_init();

  spi_init();

  i2c_init();

  sensors_init();
}

/**
 * Puts the board to sleep
 */
void board_sleep(void)
{
  nrf_pwr_mgmt_run();

/*
  // @note: Below code could be an alternative without using the power management library
  #ifdef __GNUC__
  static void __INLINE cpu_wfe(void)
  #else
  static void __forceinline cpu_wfe(void)
  #endif
  {
    __WFE();
    __SEV();
    __WFE();
  }
*/
}

/**
 * Resets the board
 */
void board_reset(void)
{
  NVIC_SystemReset();
}


//=========================== private =========================================


/**
 * Configures the user button as input source
 */
static void button_init(void)
{
}

//=========================== interrupt handlers ==============================

