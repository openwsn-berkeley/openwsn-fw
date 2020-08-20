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
                                            __disable_interrupt(); \
                                            debugpins_intdisabled_set();
   #define ENABLE_INTERRUPTS()              debugpins_intdisabled_clr(); \
                                            __set_interrupt_state(s);
#endif

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t

#define PORT_SIGNED_INT_WIDTH               int16_t
#define PORT_TICS_PER_MS                    33
#define PORT_US_PER_TICK                    30 // number of us per 32kHz clock tick
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
