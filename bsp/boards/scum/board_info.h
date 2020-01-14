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
#define US_PER_TICK                         2 // number of us per tick

#define SCHEDULER_WAKEUP()                  
#define SCHEDULER_ENABLE_INTERRUPT()

// ==== SCuM RF timer specific 

// NOTE: 
// This convert has a problem that when multiple the value, it may exceeds 
// 0xffffffff, resulting a wrong converting. Resolve this problem when the 
// frequency of rftimer is determined finally.

// this is called when require to WRITE the RFTIMER counter/compare registers,
// where the value is going to be multiplied.
#define TIMER_COUNTER_CONVERT_32K_TO_RFTIMER_CLK(value)    value*5
// this is called when require to READ  the RFTIMER counter/compare registers,
// where the value is going to be divided.
#define TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(value)    value/5

//===== radio

#define PORT_PIN_RADIO_RESET_LOW()          RFCONTROLLER_REG__CONTROL = RF_RESET;

//===== IEEE802154E timing

#define SLOTDURATION 20 // in miliseconds

//// time-slot related
#define PORT_TsSlotDuration                 9991   // 10000 ticks =   20ms  @500000hz
#define PORT_maxTxDataPrepare               1650   // 1650  ticks = 3300us  @500000hz
#define PORT_maxRxAckPrepare                504    // 504   ticks = 1006us  @500000hz
#define PORT_maxRxDataPrepare               504    // 504   ticks = 1006us  @500000hz
#define PORT_maxTxAckPrepare                1000   // 1000  ticks = 2000us  @500000hz
// radio speed related
#define PORT_delayTx                        39     //  39  ticks  =   78us  @500000hz
#define PORT_delayRx                        0      //  0us (can not measure)

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // by ticks

//===== SCuM speicification

#define SLOT_FSM_IMPLEMENTATION_SINGLE_COMPARE_TIMER_INTERRUPT
// #define SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT

// The scum setting frequency for tx and rx are seperated, 
//  the radio interface needs to be changed to adapt this feature
//  Another reason is the setting frequency takes long since we need write the 
//  frequency variables from FPGA to the PCB board. SCuM needs a seperating 
//  timing for when to send the frame which currently is a common value for all board.
// Those are the two reasons to define the marco below to insolated the SCuM related code.
// This marco shouldn't be used with the new tapeout which should be ready around July 2019.

//#define DAGROOT

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
