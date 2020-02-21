/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Modified: Tengfei Chang (tengfei.chang@eecs.berkeley.edu)
 * Date:   July 2013
 * Description: CC2538-specific board information bsp module.
 */

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include <stdint.h>
#include <string.h>

#include <source/cpu.h>
#include <source/interrupt.h>

//=========================== defines =========================================

//===== interrupt state

#define INTERRUPT_DECLARATION()
#define DISABLE_INTERRUPTS() IntMasterDisable()

#define ENABLE_INTERRUPTS() IntMasterEnable()

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33
#define PORT_US_PER_TICK                    30 // number of us per 32kHz clock tick

// on GINA, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()
#define SCHEDULER_ENABLE_INTERRUPT()

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()

/* sleep timer interrupt */
#define HAL_INT_PRIOR_ST        (4 << 5)
/* MAC interrupts */
#define HAL_INT_PRIOR_MAC       (4 << 5)
/* UART interrupt */
#define HAL_INT_PRIOR_UART      (5 << 5)

//===== pinout

// [P4.7] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()
// radio reset line
// on cc2538, the /RST line is not connected to the uC
#define PORT_PIN_RADIO_RESET_HIGH()    // nothing
#define PORT_PIN_RADIO_RESET_LOW()     // nothing

//===== IEEE802154E timing

#define SLOTDURATION 20                // in miliseconds

//===== IEEE802154E timing

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

#if SLOTDURATION==20
    #define PORT_TsSlotDuration                 655   //    20ms

    // execution speed related
    #define PORT_maxTxDataPrepare                15   //   458us (measured  213us)
    #define PORT_maxRxAckPrepare                 10   //   305us (measured   86us)
    #define PORT_maxRxDataPrepare                10   //   305us (measured   88us)
    #define PORT_maxTxAckPrepare                 15   //   458us (measured  211us)

    // radio speed related
    #define PORT_delayTx                        13    //   397us (measured  388us)
    #define PORT_delayRx                        0     //     0us (can not measure)
#endif

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//===== per-board number of sensors

#define NUMSENSORS      7

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "CC2538";
static const uint8_t infouCName[]           = "CC2538";
static const uint8_t infoRadioName[]        = "CC2538 SoC";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
