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
#define PORT_TICS_PER_MS                    32
#define SCHEDULER_WAKEUP()                  
#define SCHEDULER_ENABLE_INTERRUPT()        

// ==== SCuM RF timer specific 

// with the setup of FPGA board +teensy, the rftimer clock is 
// 20MHz divided by 255 (around 78431Hz), the following code is converting 
// between 32768Hz and 78431Hz, the ratio between them is around 2 and 5
// NOTICE: 
// 1) 255 is the maxium value can be divided. 
// 2) on FPGA, 20MHz clock can't be slow down.

// NOTE: 
// This convert has a problem that when multiple the value, it may exceeds 
// 0xffffffff, resulting a wrong converting. Resolve this problem when the 
// frequency of rftimer is determined finally.

// this is called when require to WRITE the RFTIMER counter/compare registers,
// where the value is going to be multiplied.
#define TIMER_COUNTER_CONVERT_32K_TO_RFTIMER_CLK(value)    value*5/2
// this is called when require to READ  the RFTIMER counter/compare registers,
// where the value is going to be divided.
#define TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(value)    value*2/5

//===== radio

#define PORT_PIN_RADIO_RESET_LOW()          RFCONTROLLER_REG__CONTROL = 0x10;

//===== IEEE802154E timing

#define SLOTDURATION 15 // in miliseconds

//// time-slot related
#define PORT_TsSlotDuration                 817   // 491 ticks = 15ms   @32768Hz
#define PORT_maxTxDataPrepare               100   // 66  ticks = 2013us @32768Hz
#define PORT_maxRxAckPrepare                20    // 20  ticks = 610us  @32768Hz
#define PORT_maxRxDataPrepare               33    // 33  ticks = 1006us @32768Hz
#define PORT_maxTxAckPrepare                45    // 30  ticks = 915us  @32768Hz
// radio speed related
#define PORT_delayTx                        20     //  5  ticks = 152us  @32768hz
#define PORT_delayRx                        0      //  0us (can not measure)

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       2     // by ticks

//===== SCuM speicification

#define SLOT_FSM_IMPLEMENTATION_SINGLE_COMPARE_TIMER_INTERRUPT
// #define SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT

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
