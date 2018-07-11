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
#include "sdk/components/libraries/util/nrf_assert.h"
#include "sdk/components/toolchain/cmsis/include/core_cm4.h"
#include "sdk/integration/nrfx/nrfx_glue.h"

#include "sdk/components/boards/boards.h"

#include "board.h"
#include "leds.h"
// #include "bsp_timer.h"   ///< OBSOLETE, use sctimer instead
#include "sctimer.h"
// #include "radiotimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "spi.h"
#include "radio.h"

#if (ENABLE_SEGGER_SYSVIEW == 1)
#include "SEGGER_SYSVIEW.h"
#endif // (ENABLE_SEGGER_SYSVIEW == 1)


//=========================== variables =======================================

//=========================== prototypes ======================================

static void button_init(void);

#if 0
static void clock_event_handler(nrfx_clock_evt_type_t event);
#endif

//=========================== main ============================================

extern int mote_main(void);

int main(void)
{
#if (ENABLE_SEGGER_SYSVIEW == 1)
   SEGGER_SYSVIEW_Conf();
   SEGGER_SYSVIEW_Start();
#endif // (ENABLE_SEGGER_SYSVIEW == 1)
   return mote_main();
}

//=========================== public ==========================================

void board_init(void)
{
  // start low-frequency clock (LFCLK)
  nrf_drv_clock_init();
  nrf_drv_clock_lfclk_request(NULL);
  while (!nrf_drv_clock_lfclk_is_running()) { }

  // put low-frequency clock into ultra low power (ULP) mode (will NOT work on the older nRF52832)
  // NRF_CLOCK->LFRCMODE= (CLOCK_LFRCMODE_MODE_ULP & CLOCK_LFRCMODE_MODE_Msk) << CLOCK_LFRCMODE_MODE_Pos;

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
}

/**
 * Puts the board to sleep
 */
void board_sleep(void)
{
#if (ENABLE_SEGGER_SYSVIEW == 1)
  SEGGER_SYSVIEW_OnIdle();
#endif // (ENABLE_SEGGER_SYSVIEW == 1)
  nrf_pwr_mgmt_run();

/*
  // @note: Below code could be an alternative without using the power management library
  // power management command (waiting for the next NVIC event)
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

#if 0
static void clock_event_handler(nrfx_clock_evt_type_t event)
{
  switch(event)
  {
    case NRFX_CLOCK_EVT_HFCLK_STARTED:  ///< HFCLK has been started
    // ...
    break;

    case NRFX_CLOCK_EVT_LFCLK_STARTED:  ///< LFCLK has been started
    // ...
    break;

    case NRFX_CLOCK_EVT_CTTO:           ///< Calibration timeout
    // ...
    break;

    case NRFX_CLOCK_EVT_CAL_DONE:       ///< Calibration has been done
    // ...
    break;

    
  }
}
#endif