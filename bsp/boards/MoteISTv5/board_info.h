/**
\brief MoteISTv5-specific board information bsp module.

This module file defines board-related element, but which are applicable only
to this board.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
\author Diogo Guerra <diogoguerra@ist.utl.pt>, <dy090.guerra@gmail.com>, July 2015.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "hal_MoteISTv5.h"
#include "string.h"


//=========================== define ==========================================

//===== project def
//#define   portBASE_TYPE                     uint16_t

//===== interrupt state

#if defined(__GNUC__) && (__GNUC__==4)  && (__GNUC_MINOR__<=5) && defined(__MSP430__)
   // mspgcc <4.5.x
   #define INTERRUPT_DECLARATION()          unsigned short s;
   #define DISABLE_INTERRUPTS()             s = READ_SR&0x0008; \
                                            __disable_interrupt();__nop()
   #define ENABLE_INTERRUPTS()              __asm__("bis %0,r2" : : "ir" ((uint16_t) s));__nop()
#else
   // other
   #define INTERRUPT_DECLARATION()          __istate_t s; __nop(); 
   #define DISABLE_INTERRUPTS()             s = __get_interrupt_state(); \
                                            __disable_interrupt(); __nop();
   #define ENABLE_INTERRUPTS()              __set_interrupt_state(s); __nop();
   //#define INTERRUPT_DECLARATION() ;                                      
   //#define DISABLE_INTERRUPTS()  __disable_interrupt(); __nop()
   //#define ENABLE_INTERRUPTS()   __enable_interrupt(); __nop()
#endif

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_RADIOTIMER_WIDTH               uint16_t

#define PORT_SIGNED_INT_WIDTH               int16_t 
#define PORT_TICS_PER_MS                    33

// on MoteISTv5, we use the Timer A1 CCR0 interrupt for the OS tick
#ifdef USE_FREERTOS
#define SCHEDULER_WAKEUP()                  TA1CCTL0 |= CCIFG
#define SCHEDULER_ENABLE_INTERRUPT()        TA1CCTL0  |= CCIE
#else
#define SCHEDULER_WAKEUP()                  
#define SCHEDULER_ENABLE_INTERRUPT()        
#endif

//===== pins

// [P9.7] radio VREG
#define PORT_PIN_RADIO_VREG_HIGH()          P9OUT |=  0x80;__delay_cycles(15000000); //0,6 s @25MHz
#define PORT_PIN_RADIO_VREG_LOW()           P9OUT &= ~0x80;
// [P9.6] radio RESET
#define PORT_PIN_RADIO_RESET_HIGH()         P9OUT |=  0x40;__delay_cycles(2500000);
#define PORT_PIN_RADIO_RESET_LOW()          P9OUT &= ~0x40;__delay_cycles(2500000);  

//===== IEEE802154E timing

// time-slot related
#define PORT_TsSlotDuration                 492   // counter counts one extra count, see datasheet

// execution speed related
#define PORT_maxTxDataPrepare               100   //  2899us (measured 2420us)
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
static const uint8_t infoBoardname[]        = "MoteISTv5";
static const uint8_t infouCName[]           = "MSP430f5438a";
static const uint8_t infoRadioName[]        = "CC2420";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
