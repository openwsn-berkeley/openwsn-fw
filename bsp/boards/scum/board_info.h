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

#define DISABLE_INTERRUPTS()            __disable_irq();
#define ENABLE_INTERRUPTS()             __enable_irq();

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    32
#define SCHEDULER_WAKEUP()                  
#define SCHEDULER_ENABLE_INTERRUPT()        

//===== radio

#define PORT_PIN_RADIO_RESET_LOW()          RFCONTROLLER_REG__CONTROL = 0x10;

//===== IEEE802154E timing

//// time-slot related
#define PORT_TsSlotDuration                 491   // counter counts one extra count, see datasheet
// execution speed related   (rcc configure need 235us)
#define PORT_maxTxDataPrepare               66    // 2014us (measured 812us+235) stm32
#define PORT_maxRxAckPrepare                20    //  900us (measured 171us+235) stm32
#define PORT_maxRxDataPrepare               33    //  976us (measured 170us+235) stm32
#define PORT_maxTxAckPrepare                30    //  900us (measured 323us+235) stm32

// radio speed related
#define PORT_delayTx                        24    //  549us (measured 315us+235) .....
#define PORT_delayRx                        0     //    0us (can not measure)

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                           2     // by ticks

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
