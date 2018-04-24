/**
\brief TelosB-specific board information bsp module.

This module file defines board-related element, but which are applicable only
to this board.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "msp430f1611.h"
#include "string.h"

//=========================== define ==========================================

//===== interrupt state

#if defined(__GNUC__) && (__GNUC__==4)  && (__GNUC_MINOR__<=5) && defined(__MSP430__)
   // mspgcc <4.5.x
   #define INTERRUPT_DECLARATION()          unsigned short s;
   #define DISABLE_INTERRUPTS()             s = READ_SR&0x0008; \
                                            __disable_interrupt();
   #define ENABLE_INTERRUPTS()              __asm__("bis %0,r2" : : "ir" ((uint16_t) s));
#else
   // other
   #define INTERRUPT_DECLARATION()          __istate_t s;
   #define DISABLE_INTERRUPTS()             s = __get_interrupt_state(); \
                                            __disable_interrupt();
   #define ENABLE_INTERRUPTS()              __set_interrupt_state(s);
#endif

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t

#define PORT_SIGNED_INT_WIDTH               int16_t
#define PORT_TICS_PER_MS                    33

// on TelosB, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()                  CACTL1 |= CAIFG
#define SCHEDULER_ENABLE_INTERRUPT()        CACTL1  = CAIE

//===== pins

// [P4.5] radio VREG
#define PORT_PIN_RADIO_VREG_HIGH()          P4OUT |=  0x20;
#define PORT_PIN_RADIO_VREG_LOW()           P4OUT &= ~0x20;
// [P4.6] radio RESET
#define PORT_PIN_RADIO_RESET_HIGH()         P4OUT |=  0x40;
#define PORT_PIN_RADIO_RESET_LOW()          P4OUT &= ~0x40;  

//===== IEEE802154E timing

#define SLOTDURATION 15 // in miliseconds

// time-slot related
#define PORT_TsSlotDuration                 492   // counter counts one extra count, see datasheet

// execution speed related
#define PORT_maxTxDataPrepare               100   //  2899us (measured 2516us)
#define PORT_maxRxAckPrepare                20    //   610us (measured  474us)
#define PORT_maxRxDataPrepare               33    //  1000us (measured  477us)
#define PORT_maxTxAckPrepare                45    //  1372us (measured 1328us)- cannot be bigger than 28.. is the limit for telosb as actvitiy_rt5 is executed almost there.

// radio speed related
#define PORT_delayTx                        16    //   488us (measured  473us)
#define PORT_delayRx                        0     //     0us (can not measure)

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//=========================== variables =======================================

// The variables below are used by CoAP's registration engine.

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "TelosB";
static const uint8_t infouCName[]           = "MSP430f1611";
static const uint8_t infoRadioName[]        = "CC2420";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
