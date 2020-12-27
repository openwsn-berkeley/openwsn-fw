/**
\brief nRF52840-specific board information bsp module.

This module file defines board-related element, but which are applicable only
to this board.

\author Tengfei Chang <tengfei.chang@gmail.com>, July 2020.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "string.h"
#include "nrf52833.h"
#include "core_cm4.h"
#include "cmsis_gcc.h"

//=========================== define ==========================================

//===== interrupt state

#define INTERRUPT_DECLARATION()     // todo
#define DISABLE_INTERRUPTS()        __disable_irq() 
#define ENABLE_INTERRUPTS()         __enable_irq()

//===== timer

#define PORT_TIMER_WIDTH            uint32_t    // to check

#define PORT_SIGNED_INT_WIDTH        int32_t    // to check
#define PORT_TICS_PER_MS                  33    // to check
#define PORT_US_PER_TICK                  30    // to check

#define SCHEDULER_WAKEUP()                  // todo
#define SCHEDULER_ENABLE_INTERRUPT()        // todo

//===== pins

//===== IEEE802154E timing

#define SLOTDURATION 8 // in miliseconds

// time-slot related
#define PORT_TsSlotDuration                 262 // 8ms, to check

// execution speed related
#define PORT_maxTxDataPrepare               15  // (217 us)
#define PORT_maxRxAckPrepare                10  // (147 us)
#define PORT_maxRxDataPrepare               12  // (175 us)
#define PORT_maxTxAckPrepare                15  // (228 us)

// radio speed related
#define PORT_delayTx                        1   // to check
#define PORT_delayRx                        0   // to check

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1   // ticks


// priority range from 0 (highest) ~ 7
#define RTC_PRIORITY          0 
#define EGU0_PRIORITY         0
#define RADIO_PRIORITY        0
#define UART_PRIORITY         1

//=========================== variables =======================================

// The variables below are used by CoAP's registration engine.

static const uint8_t rreg_uriquery[]        = "h=monolet";
static const uint8_t infoBoardname[]        = "nRF52840-DK";
static const uint8_t infouCName[]           = "nRF52840";
static const uint8_t infoRadioName[]        = "nRF52840-BLE";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
