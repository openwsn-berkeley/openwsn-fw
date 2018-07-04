/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific board information bsp module.
 */

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include <stdint.h>
#include <string.h>

#include "sdk/modules/nrfx/templates/nRF52840/nrfx_config.h"
// #include "sdk/components/toolchain/cmsis/include/core_cmFunc.h"
#include "opendefs.h"


//=========================== defines =========================================

//===== interrupt state

#define INTERRUPT_DECLARATION()
#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i" : : : "memory")    ///< or use __disable_irq() from core_cmFunc.h
#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i" : : : "memory")     ///< or use __enable_irq() from core_cmFunc.h

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t      ///< actually, 24 bit, but we will virtualize the 32 bit timer
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33

// on GINA, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()
#define SCHEDULER_ENABLE_INTERRUPT()

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()



//===== pinout

// [P4.7] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()
// radio reset line
// on ezr32wg, the /RST line is not connected to the uC
#define PORT_PIN_RADIO_RESET_HIGH()    // nothing
#define PORT_PIN_RADIO_RESET_LOW()     // nothing

//===== IEEE802154E timing

#define SLOTDURATION 10 // in miliseconds

#if SLOTDURATION==10
    // time-slot related
    #define PORT_TsSlotDuration                 328   // counter counts one extra count, see datasheet
    // execution speed related
    #define PORT_maxTxDataPrepare               10    //  305us (measured  82us)
    #define PORT_maxRxAckPrepare                10    //  305us (measured  83us)
    #define PORT_maxRxDataPrepare                4    //  122us (measured  22us)
    #define PORT_maxTxAckPrepare                10    //  122us (measured  94us)
    // radio speed related
    #ifdef L2_SECURITY_ACTIVE
    #define PORT_delayTx                        14    //  366us (measured xxxus)
    #else
    #define PORT_delayTx                        12    //  366us (measured xxxus)
    #endif
    #define PORT_delayRx                         0    //    0us (can not measure)
    // radio watchdog
#endif

#if SLOTDURATION==15
    // time-slot related
    #define PORT_TsSlotDuration                 492   // counter counts one extra count, see datasheet
    // execution speed related
    #define PORT_maxTxDataPrepare               66    // 2014us (measured 746us)
    #define PORT_maxRxAckPrepare                10    //  305us (measured  83us)
    #define PORT_maxRxDataPrepare               33    // 1007us (measured  84us)
    #define PORT_maxTxAckPrepare                22    //  305us (measured 219us)
    // radio speed related
    #define PORT_delayTx                        12    //  214us (measured 219us)
    #define PORT_delayRx                        0     //    0us (can not measure)
    // radio watchdog
#endif

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                           1     // ticks

//===== per-board number of sensors

#define NUMSENSORS 7

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "nRF52840DK";
static const uint8_t infouCName[]           = "nRF52840";
static const uint8_t infoRadioName[]        = "nRF52840 SoC";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
