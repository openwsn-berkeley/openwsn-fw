/**
\brief GINA-specific board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "msp430x26x.h"
#include "string.h"

//=========================== defines =========================================

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
#define PORT_RADIOTIMER_WIDTH               uint16_t

#define PORT_SIGNED_INT_WIDTH               int16_t
#define PORT_TICS_PER_MS                    33

// on GINA, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()                  CACTL1 |= CAIFG
#define SCHEDULER_ENABLE_INTERRUPT()        CACTL1  = CAIE

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()  TACCTL2 |=  CCIS0;  \
                        TACCTL2 &= ~CCIS0;

//===== pinout

// [P4.7] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()   P4OUT |=  0x80;
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()    P4OUT &= ~0x80;
// radio reset line
// on GINA, the /RST line is not connected to the uC
#define PORT_PIN_RADIO_RESET_HIGH()    // nothing
#define PORT_PIN_RADIO_RESET_LOW()     // nothing

//===== IEEE802154E timing

// time-slot related
#define PORT_TsSlotDuration                 492   // counter counts one extra count, see datasheet

// execution speed related
#define PORT_maxTxDataPrepare               66    // 2014us (measured 746us)
#define PORT_maxRxAckPrepare                10    //  305us (measured  83us)
#define PORT_maxRxDataPrepare               33    // 1007us (measured  84us)
#define PORT_maxTxAckPrepare                22    //  305us (measured 219us)
// radio speed related
#define PORT_delayTx                        7     //  214us (measured 219us)
#define PORT_delayRx                        0     //    0us (can not measure)
// radio watchdog

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "GINA";
static const uint8_t infouCName[]           = "MSP430f2618";
static const uint8_t infoRadioName[]        = "AT86RF231";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
