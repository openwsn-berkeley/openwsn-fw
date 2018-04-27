/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "board" bsp module.
 */


#include "sdk/modules/nrfx/hal/nrf_power.h"
#include "sdk/modules/nrfx/drivers/include/nrfx_clock.h"
#include "sdk/components/libraries/pwr_mgmt/nrf_pwr_mgmt.h"

#include "sdk/modules/nrfx/mdk/nrf52840.h"
#include "sdk/modules/nrfx/drivers/nrfx_common.h"
#include "sdk/components/libraries/util/nrf_assert.h"
#include "sdk/components/toolchain/cmsis/include/core_cm4.h"
#include "sdk/integration/nrfx/nrfx_glue.h"

#include "sdk/components/boards/boards.h"

#include "board.h"
#include "leds.h"
// #include "bsp_timer.h"
// #include "radiotimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "spi.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

static void button_init(void);
static void clock_event_handler(nrfx_clock_evt_type_t event);

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
  nrfx_clock_init(clock_event_handler);
  nrfx_clock_lfclk_start();

  // put low-frequency clock into ultra low power (ULP) mode (will NOT work on the older nRF52832)
  NRF_CLOCK->LFRCMODE= (CLOCK_LFRCMODE_MODE_ULP & CLOCK_LFRCMODE_MODE_Msk) << CLOCK_LFRCMODE_MODE_Pos;
  
  // enable on-board DC-DC converter to reduce overall consumption (this also disables the LDO [low-dropout] regulator)
  // This only works if the required coil and condenser are properly connected to the pins DCC and DEC4, which is the
  // case with the development kit, but not with some other nRF52840-based boards. (If enabled without the proper 
  // circuitry, the CPU will hang.)
  nrf_power_dcdcen_set(true);

  // start high frequency clock (HFCLK) ///< @todo: Revise, whether it is really needed.
  nrfx_clock_hfclk_start();

  // initialize power management library
  nrf_pwr_mgmt_init();

  // initialize boards with LEDs and buttons
  bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);
  
//   button_init();
//   debugpins_init();
//   bsp_timer_init();
//   radiotimer_init();
//   uart_init();
//   radio_init();
//   spi_init();
}

/**
 * Puts the board to sleep
 */
void board_sleep(void)
{
  nrf_pwr_mgmt_run();

/*  @note: Below code could be an alternative without using the power management library

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
