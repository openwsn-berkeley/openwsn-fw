/**
\brief eZ430-RF2500-specific board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Chuang Qian <cqian@berkeley.edu>, April 2012.

*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "io430.h"
#include "string.h"


//=========================== defines =========================================

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_RADIOTIMER_WIDTH               uint16_t

#define PORT_TICS_PER_MS                    33

// on eZ430-RF2500, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()                  CACTL1 |= CAIFG
#define SCHEDULER_ENABLE_INTERRUPT()        CACTL1  = CAIE


// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
 #define CAPTURE_TIME()  TACCTL2 |=  CCIS0;  \
                        TACCTL2 &= ~CCIS0;


//===== pinout

// [P3.0] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()   P3OUT |=  0x01;  
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()    P3OUT &= ~0x01; 

// [P3.1] radio reset line
#define PORT_PIN_RADIO_RESET_HIGH()    P3OUT |=  0x02;
#define PORT_PIN_RADIO_RESET_LOW()     P3OUT |=  0x02;

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
static const uint8_t infoBoardname[]        = "eZ430-RF2500";
static const uint8_t infouCName[]           = "MSP430F2274";
static const uint8_t infoRadioName[]        = "CC2500";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif