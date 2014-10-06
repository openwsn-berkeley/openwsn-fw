/**
\brief openmoteSTM32 board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Tengfei Chang <tengfei.chang@gmail.com>,  July 2012.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stm32f10x_lib.h"
#include "stdint.h"
#include "string.h"

//=========================== defines =========================================
//#define DEBUG_RUN_MODE
#define DEBUG_SLEEP_MODE
//#define DEBUG_STOP_MODE

//TODO in case previous declaration fails in certain compilers. Remove this 
//one if it works with GNU GCC
//#define PACK_START  _Pragma("pack(1)")
//#define PACK_END    _Pragma("pack()")

#define INTERRUPT_DECLARATION(); //no declaration

#define DISABLE_INTERRUPTS()    NVIC_SETPRIMASK();
#define ENABLE_INTERRUPTS()     NVIC_RESETPRIMASK();

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_RADIOTIMER_WIDTH               uint16_t

#define PORT_SIGNED_INT_WIDTH               int16_t
#define PORT_TICS_PER_MS                    32
#define SCHEDULER_WAKEUP()                  EXTI->SWIER |= EXTI_Line1;
#define SCHEDULER_ENABLE_INTERRUPT()        //enable in board use EXTI_Line1

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()  TACCTL2 |=  CCIS0;  \
                        TACCTL2 &= ~CCIS0;

//===== pinout

// [P4.7] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()     GPIOB->ODR |= 0X0002;
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()      GPIOB->ODR &= ~0X0002;
// radio reset line
// radio /RST
#define PORT_PIN_RADIO_RESET_HIGH()       //GPIOC->ODR |= 0X0040;// nothing
#define PORT_PIN_RADIO_RESET_LOW()        //GPIOC->ODR &= ~0X0040;// nothing

//===== IEEE802154E timing
#ifdef DEBUG_RUN_MODE
    // time-slot related
    #define PORT_TsSlotDuration                 491   // counter counts one extra count, see datasheet
    // execution speed related
    #define PORT_maxTxDataPrepare               66    // 2014us (measured 746us)
    #define PORT_maxRxAckPrepare                20    //  305us (measured  83us)
    #define PORT_maxRxDataPrepare               33    // 1007us (measured  84us)
    #define PORT_maxTxAckPrepare                30    //  305us (measured 219us)
    // radio speed related
    #define PORT_delayTx                        10     //  214us (measured 219us)
    #define PORT_delayRx                        0     //    0us (can not measure)
    // radio watchdog
#endif

#ifdef DEBUG_SLEEP_MODE
        // time-slot related
    #define PORT_TsSlotDuration                 491   // counter counts one extra count, see datasheet
    // execution speed related
    #define PORT_maxTxDataPrepare               66    // 2014us (measured 746us)
    #define PORT_maxRxAckPrepare                20    //  305us (measured  83us)
    #define PORT_maxRxDataPrepare               33    // 1007us (measured  84us)
    #define PORT_maxTxAckPrepare                30    //  305us (measured 219us)
    // radio speed related
    #define PORT_delayTx                        10     //  214us (measured 219us)
    #define PORT_delayRx                        0     //    0us (can not measure)
    // radio watchdog
#endif

#ifdef DEBUG_STOP_MODE
    //// time-slot related
    #define PORT_TsSlotDuration                 491   // counter counts one extra count, see datasheet
    // execution speed related   (rcc configure need 235us)
    #define PORT_maxTxDataPrepare               66    // 2014us (measured 812us+235) stm32
    #define PORT_maxRxAckPrepare                20    //  900us (measured 171us+235) stm32
    #define PORT_maxRxDataPrepare               33    //  976us (measured 170us+235) stm32
    #define PORT_maxTxAckPrepare                30    //  900us (measured 323us+235) stm32
    
    // radio speed related
    #define PORT_delayTx                        24    //  549us (measured 315us+235) .....
    #define PORT_delayRx                        0     //    0us (can not measure)
#endif

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                           2     // by ticks

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "OPENMOTESTM32";
static const uint8_t infouCName[]           = "STM32F103";
static const uint8_t infoRadioName[]        = "AT86RF231";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif