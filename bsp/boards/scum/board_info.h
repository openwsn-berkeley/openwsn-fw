/**
\brief SCuM board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Tengfei Chang <tengfei.chang@inria.com>,  August 2016.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "string.h"

//=========================== defines =========================================

#define INTERRUPT_DECLARATION()

#define DISABLE_INTERRUPTS()                __disable_irq();
#define ENABLE_INTERRUPTS()                 __enable_irq();

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    32
#define SCHEDULER_WAKEUP()                  
#define SCHEDULER_ENABLE_INTERRUPT()        

// ==== SCuM RF timer specific 

// since SCuM uses 500KHz, but the statck protocol is designed for 32KHz.
// the following two marco are used to convert the couter value bewteen
// two different frequency clocks. 500000/32768 is approximately 61/4.
#define TIMER_COUTER_CONVERT_32K_TO_500K(value)    value*61/4
#define TIMER_COUTER_CONVERT_500K_TO_32K(value)    value*4/61

// ==== SCuM RF timer compare control bit

#define RFTIMER_COMPARE_ENABLE              0x01
#define RFTIMER_COMPARE_INTERRUPT_ENABLE    0x02
#define RFTIMER_COMPARE_TX_LOAD_ENABLE      0x04
#define RFIMER_COMPARE_TX_SEND_ENABLE       0x08
#define RFTIMER_COMPARE_RX_START_ENABLE     0x10
#define RFTIMER_COMPARE_RX_STOP_ENABLE      0x20

//===== radio

#define PORT_PIN_RADIO_RESET_LOW()          RFCONTROLLER_REG__CONTROL = 0x10;

//===== IEEE802154E timing

//// time-slot related
#define PORT_TsSlotDuration                 491   // 491 ticks = 15ms   @32768Hz
#define PORT_maxTxDataPrepare               66    // 66  ticks = 2013us @32768Hz
#define PORT_maxRxAckPrepare                20    // 20  ticks = 610us  @32768Hz
#define PORT_maxRxDataPrepare               33    // 33  ticks = 1006us @32768Hz
#define PORT_maxTxAckPrepare                30    // 30  ticks = 915us  @32768Hz
// radio speed related
#define PORT_delayTx                        5     //  5  ticks = 152us  @32768hz
#define PORT_delayRx                        0     //  0us (can not measure)

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       2     // by ticks

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "SCuM";
static const uint8_t infouCName[]           = "CortexM0";
static const uint8_t infoRadioName[]        = "SCuM";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
