/**
\brief Python-specific board information bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "string.h"

//=========================== defines =========================================

#define INTERRUPT_DECLARATION()             ;
#define ENABLE_INTERRUPTS()                 ;
#define DISABLE_INTERRUPTS()                ;

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33
#define PORT_US_PER_TICK                    30 // number of us per 32kHz clock tick

// on GINA, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()
#define SCHEDULER_ENABLE_INTERRUPT()

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1 // when using openmoteSTM, change to 2

//===== sctimer scheduling
#define TIMERTHRESHOLD                     10  

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "Python";
static const uint8_t infouCName[]           = "Python";
static const uint8_t infoRadioName[]        = "Python";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif