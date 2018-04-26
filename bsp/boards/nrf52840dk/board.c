/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "board" bsp module.
 */


#include "sdk/modules/nrfx/mdk/nrf52840.h"
#include "sdk/modules/nrfx/drivers/nrfx_common.h"
#include "sdk/components/libraries/util/nrf_assert.h"
#include "sdk/integration/nrfx/nrfx_glue.h"
#include "sdk/components/toolchain/cmsis/include/core_cm4.h"


#include "board.h"
#include "leds.h"
// #include "bsp_timer.h"
// #include "radiotimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
// #include "em_cmu.h"
// #include "em_chip.h"
// #include "em_emu.h"
#include "spi.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

static void button_init(void);


//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void)
{
//   CHIP_Init();

   leds_init();
   debugpins_init();
   button_init();
//   bsp_timer_init();
//   radiotimer_init();
   uart_init();
   radio_init();
//   spi_init();
}

/**
 * Puts the board to sleep
 */
void board_sleep(void) {
    //EMU_EnterEM1();
}

/**
 * Resets the board
 */
void board_reset(void) {
	NVIC_SystemReset();
}


//=========================== private =========================================


/**
 * Configures the user button as input source
 */
static void button_init(void) {

}
//=========================== interrupt handlers ==============================

