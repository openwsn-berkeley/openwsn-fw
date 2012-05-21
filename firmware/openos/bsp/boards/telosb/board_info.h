/**
\brief TelosB-specific board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "msp430f1611.h"

//=========================== define ==========================================
//processor scpecific

#define port_INLINE                         inline

#define ENABLE_INTERRUPTS()     __enable_interrupt()
#define DISABLE_INTERRUPTS()    __disable_interrupt()

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_SIGNED_INT_WIDTH               int16_t
#define PORT_TICS_PER_MS                    33

// on TelosB, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()                  CACTL1 |= CAIFG
#define SCHEDULER_ENABLE_INTERRUPT()        CACTL1  = CAIE

//===== pinout

// [P4.5] radio VREG
#define PORT_PIN_RADIO_VREG_HIGH()          P4OUT |=  0x20;
#define PORT_PIN_RADIO_VREG_LOW()           P4OUT &= ~0x20;
// [P4.6] radio RESET
#define PORT_PIN_RADIO_RESET_HIGH()         P4OUT |=  0x40;
#define PORT_PIN_RADIO_RESET_LOW()          P4OUT &= ~0x40;  

//===== IEEE802154E timing

// time-slot related
#define PORT_TsSlotDuration                 491   // counter counts one extra count, see datasheet
// execution speed related
#define PORT_maxTxDataPrepare               95    //  2899us (measured 2420us)
#define PORT_maxRxAckPrepare                20    //   610us (measured  474us)
#define PORT_maxRxDataPrepare               33    //  1000us (measured  477us)
#define PORT_maxTxAckPrepare                24    //   732us (measured  693us)
// radio speed related
#define PORT_delayTx                        12    //   366us (measured  352us)
#define PORT_delayRx                        0     //     0us (can not measure)
// radio watchdog

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "TelosB";
static const uint8_t infouCName[]           = "MSP430f1611";
static const uint8_t infoRadioName[]        = "CC2420";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
