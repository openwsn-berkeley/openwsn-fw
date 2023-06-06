/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific board information bsp module.
 */

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include <stdint.h>
#include <string.h>
#include "cmsis_gcc.h"

//=========================== defines =========================================

//===== interrupt state

#define INTERRUPT_DECLARATION()
#define DISABLE_INTERRUPTS()  __disable_irq()
#define ENABLE_INTERRUPTS()   __enable_irq()

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t      ///< actually, 24 bit, but we will virtualize the 32 bit timer
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33
#define PORT_US_PER_TICK                    30  // number of us per 32kHz clock tick
#define PORT_MAX_TICKS_IN_SINGLE_CLOCK      (uint32_t)(0x00ffffff)

// on GINA, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()
#define SCHEDULER_ENABLE_INTERRUPT()

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()


// pin selection
#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

// interrupt priority
#define RTC_PRIORITY    0
#define RADIO_PRIORITY  0
#define UART_PRIORITY   2

//===== IEEE802154E timing
// 1 clock tick = 30.5 us

#define SLOTDURATION 20 // in miliseconds

#if SLOTDURATION==10
    // time-slot related
    #define PORT_TsSlotDuration                 328   // counter counts one extra count, see datasheet

#if BOARD_PCA10056
// nrf52840-DK
    #define PORT_maxTxDataPrepare               13    // ~397us (measured 364us)
    #define PORT_maxRxAckPrepare                13    // ~397us (measured 364us)
    #define PORT_maxRxDataPrepare               13    // ~397us (measured 364us)
    #define PORT_maxTxAckPrepare                13    // ~397us (measured 364us)

    // radio speed related
    #define PORT_delayTx                        10    //  305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    #define PORT_delayRx                         5    // ~153us (measured 147us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
#endif
#if BOARD_PCA10059
// nrf52840-DONGLE
    #define PORT_maxTxDataPrepare               13    // ~397us (measured 345us)
    #define PORT_maxRxAckPrepare                13    // ~397us (measured 345us)
    #define PORT_maxRxDataPrepare               13    // ~397us (measured 345us)
    #define PORT_maxTxAckPrepare                13    // ~397us (measured 345us)

    // radio speed related
    #define PORT_delayTx                        10    //  305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    #define PORT_delayRx                         5    // ~153us (measured 136us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
#endif

#endif // SLOTDURATION==10

#if SLOTDURATION==15
    // time-slot related
    #define PORT_TsSlotDuration                 492   // counter counts one extra count, see datasheet

#if BOARD_PCA10056
// nrf52840-DK
    #define PORT_maxTxDataPrepare               13    // ~397us (measured 364us)
    #define PORT_maxRxAckPrepare                13    // ~397us (measured 364us)
    #define PORT_maxRxDataPrepare               13    // ~397us (measured 364us)
    #define PORT_maxTxAckPrepare                13    // ~397us (measured 364us)

    // radio speed related
    #define PORT_delayTx                        10    //  305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    #define PORT_delayRx                         5    // ~153us (measured 147us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
#endif
#if BOARD_PCA10059
// nrf52840-DONGLE
    #define PORT_maxTxDataPrepare               13    // ~397us (measured 345us)
    #define PORT_maxRxAckPrepare                13    // ~397us (measured 345us)
    #define PORT_maxRxDataPrepare               13    // ~397us (measured 345us)
    #define PORT_maxTxAckPrepare                13    // ~397us (measured 345us)

    // radio speed related
    #define PORT_delayTx                        10    //  305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    #define PORT_delayRx                         5    // ~153us (measured 136us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
#endif

#endif // SLOTDURATION==15


#if SLOTDURATION==20
    // time-slot related
    #define PORT_TsSlotDuration                 656   // counter counts one extra count, see datasheet

#if BOARD_PCA10056
// nrf52840-DK
    #define PORT_maxTxDataPrepare               13    // ~397us (measured 364us)
    #define PORT_maxRxAckPrepare                13    // ~397us (measured 364us)
    #define PORT_maxRxDataPrepare               13    // ~397us (measured 364us)
    #define PORT_maxTxAckPrepare                13    // ~397us (measured 364us)

    // radio speed related
    #define PORT_delayTx                         8    //  305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    #define PORT_delayRx                         0    // ~153us (measured 147us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
#endif
#if BOARD_PCA10059
// nrf52840-DONGLE
    #define PORT_maxTxDataPrepare               13    // ~397us (measured 345us)
    #define PORT_maxRxAckPrepare                13    // ~397us (measured 345us)
    #define PORT_maxRxDataPrepare               13    // ~397us (measured 345us)
    #define PORT_maxTxAckPrepare                13    // ~397us (measured 345us)

    // radio speed related
    #define PORT_delayTx                        10    //  305us (measured 282us; radio_txNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
    #define PORT_delayRx                         5    // ~153us (measured 136us; radio_rxNow() to RADIO_IRQHandler() / NRF_RADIO->EVENTS_READY)
#endif

#endif // SLOTDURATION==20

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                            1     // ticks

//===== per-board number of sensors

#define NUMSENSORS 2

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "nRF52840";
static const uint8_t infouCName[]           = "nRF52840";
static const uint8_t infoRadioName[]        = "nRF52840 SoC";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
