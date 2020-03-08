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

#define SLOTDURATION 10 // in miliseconds
                        
//===== IEEE802154E timing
// time-slot related
#define PORT_TsSlotDuration                328    // counter counts one extra count, see datasheet
// execution speed related
#define PORT_maxTxDataPrepare               10    //  305us (measured  82us)
#define PORT_maxRxAckPrepare                10    //  305us (measured  83us)
#define PORT_maxRxDataPrepare                4    //  122us (measured  22us)
#define PORT_maxTxAckPrepare                 4    //  122us (measured  94us)
// radio speed related
#define PORT_delayTx                         7    //  366us (measured xxxus)
#define PORT_delayRx                         0    //    0us (can not measure)

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