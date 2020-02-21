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

#define DISABLE_INTERRUPTS()                __asm__( "MOVS   R0, #1;" \
                                                     "MSR    PRIMASK, R0;");
#define ENABLE_INTERRUPTS()                 __asm__( "MOVS   R0, #0;" \
                                                     "MSR    PRIMASK, R0;");

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    500
#define PORT_US_PER_TICK                    2 // number of us per 500kHz clock tick

#define SCHEDULER_WAKEUP()                  
#define SCHEDULER_ENABLE_INTERRUPT()

//===== radio

#define PORT_PIN_RADIO_RESET_LOW()          RFCONTROLLER_REG__CONTROL = RF_RESET;

//===== IEEE802154E timing

#define SLOTDURATION 20 // in miliseconds

//// time-slot related
#define PORT_TsSlotDuration                 9991   // 10000 ticks =   20ms  @500000hz
#define PORT_maxTxDataPrepare               1650   // 1650  ticks = 3300us  @500000hz
#define PORT_maxRxAckPrepare                504    // 504   ticks = 1006us  @500000hz
#define PORT_maxRxDataPrepare               504    // 504   ticks = 1006us  @500000hz
#define PORT_maxTxAckPrepare                1500   // 1500  ticks = 3000us  @500000hz
// radio speed related
#define PORT_delayTx                        39     //  39  ticks  =   78us  @500000hz
#define PORT_delayRx                        0      //  0us (can not measure)

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // by ticks

//===== SCuM speicification

#define SLOT_FSM_IMPLEMENTATION_SINGLE_COMPARE_TIMER_INTERRUPT
// #define SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT

#define RAM_SIZE_64KB

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "SCuM3C";
static const uint8_t infouCName[]           = "CortexM0";
static const uint8_t infoRadioName[]        = "SCuM";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
