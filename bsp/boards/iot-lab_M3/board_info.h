/**
\brief iot-lab_M3 board information bsp module (based on openmoteSTM32 code).

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Tengfei Chang <tengfei.chang@gmail.com>,  July 2012.
\author Alaeddine Weslati <alaeddine.weslati@inria.fr>,  January 2014.
*/

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stm32f10x_conf.h"
#include "stdint.h"
#include "string.h"

//=========================== defines =========================================

//TODO in case previous declaration fails in certain compilers. Remove this 
//one if it works with GNU GCC
//#define PACK_START  _Pragma("pack(1)")
//#define PACK_END    _Pragma("pack()")

#define INTERRUPT_DECLARATION(); //no declaration
#define DISABLE_INTERRUPTS()    __disable_irq();
#define ENABLE_INTERRUPTS()     __enable_irq();

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    32
#define PORT_US_PER_TICK                    30 // number of us per 32kHz clock tick
#define SCHEDULER_WAKEUP()                  EXTI->SWIER |= EXTI_Line1;
#define SCHEDULER_ENABLE_INTERRUPT()        //enable in board use EXTI_Line1

//===== pinout

// [PA.2] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()     GPIOA->ODR |= (1<<2);
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()      GPIOA->ODR &= ~(1<<2);
// radio reset line
// radio /RST
#define PORT_PIN_RADIO_RESET_HIGH()       //GPIOC->ODR |= 0X0040;// nothing
#define PORT_PIN_RADIO_RESET_LOW()        //GPIOC->ODR &= ~0X0040;// nothing
//#define PORT_PIN_RADIO_RESET_LOW()            GPIOC->ODR &= ~(1<<1);

//===== IEEE802154E timing

#define SLOTDURATION 20 // in miliseconds

// time-slot related
#define PORT_TsSlotDuration                 655   //    20ms

// execution speed related
#define PORT_maxTxDataPrepare               110   //  3355us (not measured)
#define PORT_maxRxAckPrepare                20    //   610us (not measured)
#define PORT_maxRxDataPrepare               33    //  1000us (not measured)
#define PORT_maxTxAckPrepare                50    //  1525us (not measured)

// radio speed related
#define PORT_delayTx                        18    //   549us (not measured)
#define PORT_delayRx                        0     //     0us (can not measure)
// radio watchdog

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       2     // ticks

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "IOT-LAB_M3";
static const uint8_t infouCName[]           = "STM32F103";
static const uint8_t infoRadioName[]        = "AT86RF231";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
