/**
\brief derfmega board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.


\author Kevin Weekly <kweekly@eecs.berkeley.edu>, June 2012.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include <avr/io.h>
#include <string.h>
#include "stdint.h"

//=========================== defines =========================================

#define PORT_SIGNED_INT_WIDTH				int32_t

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_TICS_PER_MS                    33
#define PORT_US_PER_TICK                    30 // number of us per 32kHz clock tick
/*
#define SCHEDULER_WAKEUP()                  radiotimer_isr()
#define SCHEDULER_ENABLE_INTERRUPT()        TIMSK2 |= (1<<OCIE2A)
*/
/*
#define ENABLE_INTERRUPTS()					 __asm__ __volatile__ ("sei" ::: "memory")
#define DISABLE_INTERRUPTS()				 __asm__ __volatile__ ("cli" ::: "memory")
*/
#define ENABLE_INTERRUPTS()					
#define DISABLE_INTERRUPTS()			

#define SCHEDULER_WAKEUP()                  //do nothing
#define SCHEDULER_ENABLE_INTERRUPT()        // do nothing

//===== IEEE802154E timing

// time-slot related
#define PORT_TsSlotDuration                 491   // counter counts one extra count, see datasheet
// execution speed related
#define PORT_maxTxDataPrepare               66    // 2014us (measured 746us)
#define PORT_maxRxAckPrepare                10    //  305us (measured  83us)
#define PORT_maxRxDataPrepare               33    // 1007us (measured  84us)
#define PORT_maxTxAckPrepare                10    //  305us (measured 219us)
// radio speed related
#define PORT_delayTx                        7     //  214us (measured 219us)
#define PORT_delayRx                        0     //    0us (can not measure)
// radio watchdog

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "derfmega";
static const uint8_t infouCName[]           = "ATMEGA128RFA1";
static const uint8_t infoRadioName[]        = "INTERNAL";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif