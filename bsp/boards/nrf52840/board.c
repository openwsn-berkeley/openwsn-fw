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

#include "config.h"
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

int main(void) {
    return mote_main();
}


//=========================== public ==========================================

void board_init(void) {

    // start low-frequency clock (LFCLK)
    nrf_drv_clock_init();
    NRF_CLOCK->EVENTS_LFCLKSTARTED= 0;  ///< part of workaround for 3.1 [20] RTC: Register values are invalid from http://infocenter.nordicsemi.com/pdf/nRF52840_Rev_1_Errata_v1.1.pdf
    nrf_drv_clock_lfclk_request(NULL);
    while (!nrf_drv_clock_lfclk_is_running());
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

    board_init_slot_vars();

#if defined(BOARD_SENSORS_ENABLED)
    sensors_init();
#endif
}

//====  IEEE802154E timing: bootstrapping slot info lookup table
// 1 clock tick = 30.5 us
void board_init_slot_vars(void){


    //10ms slot
    slot_board_vars [SLOT_10ms_24GHZ].slotDuration                   = 328    ;  // tics
    slot_board_vars [SLOT_10ms_24GHZ].maxTxDataPrepare               = 13     ;  // ~397us (measured 364us)
    slot_board_vars [SLOT_10ms_24GHZ].maxRxAckPrepare                = 13     ;  // ~397us (measured 364us)
    slot_board_vars [SLOT_10ms_24GHZ].maxRxDataPrepare               = 13     ;  // ~397us (measured 364us)
    slot_board_vars [SLOT_10ms_24GHZ].maxTxAckPrepare                = 13     ;  // ~397us (measured 364us)                                                                
    slot_board_vars [SLOT_10ms_24GHZ].delayTx                        = 10     ;  //  305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    slot_board_vars [SLOT_10ms_24GHZ].delayRx                        =  5     ;  // ~153us (measured 147us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)

    // 20ms slot
    slot_board_vars [SLOT_20ms_24GHZ].slotDuration                   =  655   ; // tics  
    slot_board_vars [SLOT_20ms_24GHZ].maxTxDataPrepare               =  13    ; // ~397us (measured 364us)
    slot_board_vars [SLOT_20ms_24GHZ].maxRxAckPrepare                =  13    ; // ~397us (measured 364us)
    slot_board_vars [SLOT_20ms_24GHZ].maxRxDataPrepare               =  13    ; // ~397us (measured 364us)
    slot_board_vars [SLOT_20ms_24GHZ].maxTxAckPrepare                =  13    ; // ~397us (measured 364us)
    #if BOARD_PCA10056
    // nrf52840-DK
        slot_board_vars [SLOT_20ms_24GHZ].delayTx                    =  1     ; // 305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
        slot_board_vars [SLOT_20ms_24GHZ].delayRx                    =  0     ; // ~153us (measured 147us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    #endif
    #if BOARD_PCA10059
    // nrf52840-DONGLE
        slot_board_vars [SLOT_20ms_24GHZ].delayTx                    = 10     ; // 305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
        slot_board_vars [SLOT_20ms_24GHZ].delayRx                    =  5     ; // ~153us (measured 147us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    #endif
}

// To get the current slotDuration at any time (in tics)
// if you need the value in MS, divide by PORT_TICS_PER_MS (which varies by board and clock frequency and defined in board_info.h)
uint16_t board_getSlotDuration (void){
    return slot_board_vars [selected_slot_type].slotDuration;
}

// Setter/Getter function for slot_board_vars
slot_board_vars_t board_selectSlotTemplate (slotType_t slot_type){
    selected_slot_type = slot_type;
    return slot_board_vars [selected_slot_type];
}

/**
 * Puts the board to sleep
 */
void board_sleep(void) {
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
void board_reset(void) {
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

