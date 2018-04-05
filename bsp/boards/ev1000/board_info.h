/**
\brief openmoteSTM32 board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Tengfei Chang <tengfei.chang@gmail.com>,  July 2012.
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

#define SLOTDURATION_10MS
#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    32

#define SCHEDULER_WAKEUP()                  EXTI->SWIER |= EXTI_Line1;
#define SCHEDULER_ENABLE_INTERRUPT()        //enable in board use EXTI_Line1
/*
//this is a workaround from the fact that the interrupt pin for the GINA radio
//is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()  TACCTL2 |=  CCIS0;  \
                        TACCTL2 &= ~CCIS0;
*/
//===== pinout

// [P4.7] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()//     GPIOB->ODR |= 0X0002;
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()//      GPIOB->ODR &= ~0X0002;
// radio reset line
// radio /RST
#define PORT_PIN_RADIO_RESET_HIGH()       //GPIOC->ODR |= 0X0040;
#define PORT_PIN_RADIO_RESET_LOW()        //GPIOC->ODR &= ~0X0040;

//===== IEEE802154E timing

// time-slot related
#define PORT_TsSlotDuration                 800   // ~20ms
#define PORT_TsTxOffset                     100  //  8000s
#define PORT_TsLongGT                  		43   //  1344us
#define PORT_TsTxAckDelay                   72   //  2250us
#define PORT_TsShortGT                      16   //   500us

// execution speed related
#define PORT_maxTxDataPrepare               66   // ~2ms (measured )
#define PORT_maxRxAckPrepare                20   //  us (measured us)
#define PORT_maxRxDataPrepare               33   //  us (measured us)
#define PORT_maxTxAckPrepare                30   //  us (measured us)
// radio speed related
#define PORT_delayTx                        2     //  64us (measured 31us)
#define PORT_delayRx                        0     //    0us (can not measure)
// radio watchdog
#define PORT_wdRadioTx					    32    // ~1ms
#define PORT_wdDataDuration	                384	  // 12ms  (measured 11.1ms)
#define PORT_wdAckDuration	                98
//===== adaptive_sync accuracy

#define SYNC_ACCURACY                           2     // by ticks

//=========================== typedef  ========================================

//=========================== variables =======================================

const uint8_t * const rreg_uriquery;
const uint8_t * const infoBoardname;
const uint8_t * const infouCName;
const uint8_t * const infoRadioName;

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
