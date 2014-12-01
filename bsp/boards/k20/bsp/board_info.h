/**
 \brief K20-specific board information bsp module.

 This module simply defines some strings describing the board, which CoAP uses
 to return the board's description.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#ifndef _BOARD_INFO_H
#define _BOARD_INFO_H

#include "common.h"
#include <stdint.h>

//=========================== defines =========================================

#define ID 0x45

#define PORT_TICS_PER_MS                    281
#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_RADIOTIMER_WIDTH               uint16_t

#define PORT_SIGNED_INT_WIDTH               int16_t

#define CAPTURE_TIME()   //the timer does not have a capture register so do nothing

#define INTERRUPT_DECLARATION()    //do nothing by now.
#define DISABLE_INTERRUPTS() DisableInterrupts

#define ENABLE_INTERRUPTS() EnableInterrupts


#define SCHEDULER_WAKEUP()                  //do nothing
#define SCHEDULER_ENABLE_INTERRUPT()        //do nothing.


//===== IEEE802154E timing

// time-slot related
#define PORT_TsSlotDuration                 490   // counter counts one extra count, see datasheet
// execution speed related
#define PORT_maxTxDataPrepare               77    // 2014us (measured 746us)
#define PORT_maxRxAckPrepare                12    //  305us (measured  83us)
#define PORT_maxRxDataPrepare               36    // 1007us (measured  84us)
#define PORT_maxTxAckPrepare                12    //  305us (measured 219us)
// radio speed related
#define PORT_delayTx                        12     //  214us (measured 219us)
#define PORT_delayRx                        0     //    0us (can not measure)
// radio watchdog

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1    // ticks 


//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "k20";
static const uint8_t infouCName[]    = "Freescale k20DX72";
static const uint8_t infoRadioName[] = "AT86RF231";

//=========================== defines =========================================



// SLP_TR PTB3 -- pin B51
#ifdef TOWER_K20
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()  GPIOB_PSOR |= RADIO_SLPTR_MASK //set
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()   GPIOB_PCOR |= RADIO_SLPTR_MASK //clear

//  radio RSTn PTC8
#define PORT_PIN_RADIO_RESET_HIGH()     GPIOC_PCOR |= RADIO_RST_MASK; //clear as it is inverted. 
#define PORT_PIN_RADIO_RESET_LOW()      GPIOC_PSOR |= RADIO_RST_MASK; //set to high (as radio pin is inverted so this means no reset.)


#define RADIO_SLPTR_PIN 3 //PTB4
#define RADIO_SLPTR_MASK (1<<RADIO_SLPTR_PIN)
#define RADIO_ISR_PIN 5  //PTC5
#define RADIO_ISR_MASK (1<<RADIO_ISR_PIN)
#define RADIO_RST_PIN 9 //PTC9 
#define RADIO_RST_MASK (1<<RADIO_RST_PIN)

#elif OPENMOTE_K20
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()  GPIOD_PSOR |= RADIO_SLPTR_MASK //set
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()   GPIOD_PCOR |= RADIO_SLPTR_MASK //clear

//  radio RSTn PTD5
#define PORT_PIN_RADIO_RESET_HIGH()       GPIOD_PCOR |= RADIO_RST_MASK; //clear as it is inverted. 
#define PORT_PIN_RADIO_RESET_LOW()        GPIOD_PSOR |= RADIO_RST_MASK; //set to high (as radio pin is inverted so this means no reset.)


#define RADIO_SLPTR_PIN 4 //PTD4
#define RADIO_SLPTR_MASK (1<<RADIO_SLPTR_PIN)
#define RADIO_ISR_PIN 5  //PTC5
#define RADIO_ISR_MASK (1<<RADIO_ISR_PIN)
#define RADIO_RST_PIN 5 //PTD5  
#define RADIO_RST_MASK (1<<RADIO_RST_PIN)

#endif

/*
 MK20D72 NVIC isr numbers - pag 65-68 manual.
 */
#define LLWU_IRQ_NUM                             21
#define SPI0_IRQ_NUM                             26
#define UART1_IRQ_NUM                            47 //see page 67 of the manual
#define LPTMR_IRQ_NUM                            85
#define RADIO_EXTERNAL_PORT_IRQ_NUM              89 //vector 105


#endif /* _BOARD_INFO_H */
