/**
\brief WSN430v13b-specific board information bsp module.

This module file defines board-related element, but which are applicable only
to this board.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Adilla Susungi <adilla.susungi@etu.unistra.fr>, August 2013.
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
#define PORT_RADIOTIMER_WIDTH               uint16_t

#define PORT_SIGNED_INT_WIDTH               int16_t
#define PORT_TICS_PER_MS                    33

// on WSN430v13b, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()                  CACTL1 |= CAIFG
#define SCHEDULER_ENABLE_INTERRUPT()        CACTL1  = CAIE

//===== pins


// [P3.1] SIMO
#define PORT_PIN_SIMO_HIGH()                P3OUT |=  0x02;
#define PORT_PIN_SIMO_LOW()                 P3OUT &= ~0x02;
// [P3.2] SOMI
#define PORT_PIN_SOMI_HIGH()                P3IN  |=  0x04;
#define PORT_PIN_SOMI_LOW()                 P3IN  &= ~0x04;
// [P3.3] SCLK
#define PORT_PIN_SCLK_HIGH()                P3OUT |=  0x08;
#define PORT_PIN_SCLK_LOW()                 P3OUT &= ~0x08;
// [P4.2] CSn
#define PORT_PIN_CS_HIGH()                  P4OUT |=  0x04;
#define PORT_PIN_CS_LOW()                   P4OUT &= ~0x04;
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
#define PORT_maxTxDataPrepare               100    //  2899us (measured 2420us)
#define PORT_maxRxAckPrepare                20    //   610us (measured  474us)
#define PORT_maxRxDataPrepare               33    //  1000us (measured  477us)
#define PORT_maxTxAckPrepare                40    //   792us (measured  746us)- cannot be bigger than 28.. is the limit for telosb as actvitiy_rt5 is executed almost there.

// radio speed related
#define PORT_delayTx                        12    //   366us (measured  352us)
#define PORT_delayRx                        0     //     0us (can not measure)

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//=========================== variables =======================================

// The variables below are used by CoAP's registration engine.

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "WSN430v13b";
static const uint8_t infouCName[]           = "MSP430f1611";
static const uint8_t infoRadioName[]        = "CC1101";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
