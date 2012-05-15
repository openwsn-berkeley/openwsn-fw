/**
 \brief K20-specific board information bsp module.

 This module simply defines some strings describing the board, which CoAP uses
 to return the board's description.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#ifndef _BOARD_INFO_H
#define _BOARD_INFO_H

#include "derivative.h"

#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_TICS_PER_MS                    33


#define CAPTURE_TIME()   //the timer does not have a capture register so do nothing
#define DISABLE_INTERRUPTS() DisableInterrupts

#define ENABLE_INTERRUPTS() EnableInterrupts



#define SCHEDULER_WAKEUP()                  //do nothing
#define SCHEDULER_ENABLE_INTERRUPT()        //do nothing.


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


//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "k20";
static const uint8_t infouCName[]    = "Freescale k20DX72";
static const uint8_t infoRadioName[] = "AT86RF231";

//=========================== defines =========================================



// SLP_TR 

#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()  
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()   

//  radio RSTn 
#define PORT_PIN_RADIO_RESET_HIGH()       
#define PORT_PIN_RADIO_RESET_LOW()     


/*
 MK20D72 NVIC isr numbers - pag 65-68 manual.
 */
#define LLWU_IRQ_NUM                             21
#define SPI0_IRQ_NUM                             26
#define UART1_IRQ_NUM                            47 //see page 67 of the manual
#define LPTMR_IRQ_NUM                            85



/*
   ISR prototypes
 */

void lptmr_isr(void);
void llwu_isr(void);
void uart_isr(void);
extern uint8_t spi_isr(void);  //defined in spi.h. for compatibility reasons is kept there.

#endif /* _BOARD_INFO_H */
