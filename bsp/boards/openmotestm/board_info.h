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

//TODO in case previous declaration fails in certain compilers. Remove this 
//one if it works with GNU GCC
//#define PACK_START  _Pragma("pack(1)")
//#define PACK_END    _Pragma("pack()")

#define INTERRUPT_DECLARATION(); //no declaration

#define DISABLE_INTERRUPTS()    NVIC_SETPRIMASK();
#define ENABLE_INTERRUPTS()     NVIC_RESETPRIMASK();

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    32
#define PORT_US_PER_TICK                    30 // number of us per 32kHz clock tick
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
#define PORT_PIN_RADIO_RESET_HIGH()       //GPIOC->ODR |= 0X0040;
#define PORT_PIN_RADIO_RESET_LOW()        //GPIOC->ODR &= ~0X0040;


//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       2     // ticks

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