/**
 \brief LPC1769-specific board information bsp module.

 This module simply defines some strings describing the board, which CoAP uses
 to return the board's description.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#ifndef _BOARD_INFO_H
#define _BOARD_INFO_H

#include "string.h"
#include "debugpins.h"
#include "stdint.h"
#include "LPC17xx.h"
#include "lpc_types.h"

//=========================== defines =========================================

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t
#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33

//P0.23 is CAP3.0 (capture register for the radio timer)
#define CAPTURE_TIME()   LPC_GPIO0->FIOSET        |=  CAPTURE_PIN_MASK;  \
                         LPC_GPIO0->FIOCLR        |=  CAPTURE_PIN_MASK;

#define CAPTURE_PIN_MASK 1<<15  //GPIO P0.15 to capture

#define DISABLE_INTERRUPTS() __disable_irq(); \
                      debugpins_isr_set();

#define ENABLE_INTERRUPTS() __enable_irq(); \
                            debugpins_isr_clr();



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

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks


//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "xpressohack";
static const uint8_t infouCName[]    = "NXP LPC1769";
static const uint8_t infoRadioName[] = "AT86RF231";

//=========================== defines =========================================



//#define SPI_IN_INTERRUPT_MODE
//#define SPI_IN_RTOS_MODE

// SLP_TR [P1.22]
#ifdef OPENMOTE
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()   LPC_GPIO1->FIOSET |=  1<<22;
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()    LPC_GPIO1->FIOCLR |=  1<<22;
#define RADIO_ISR_MASK 0x1<<22
#endif

// SLP_TR [P2.8]
#ifdef LPCXPRESSO1769
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()   LPC_GPIO2->FIOSET |=  1<<8;
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()    LPC_GPIO2->FIOCLR |=  1<<8;
#define RADIO_ISR_MASK 0x1<<21
#endif


// [P2.4] radio RSTn [P0.17]
//#define PORT_PIN_RADIO_RESET_HIGH()         LPC_GPIO0->FIOSET |=  1<<17;
//#define PORT_PIN_RADIO_RESET_LOW()          LPC_GPIO0->FIOCLR |=  1<<17;

#define PORT_PIN_RADIO_RESET_HIGH()         LPC_GPIO0->FIOCLR |=  1<<17; //as Radio RST is negated it is cleared here
#define PORT_PIN_RADIO_RESET_LOW()          LPC_GPIO0->FIOSET |=  1<<17;

//isr radio is GPIO  P0.22

//===== peripheral fw library configuratoin definitions

// Comment the line below to disable the specific peripheral inclusion

// DEBUG_FRAMWORK
#define _DBGFWK
// GPIO
#define _GPIO
// EXTI
#define _EXTI
// UART
#define _UART
#define _UART0
#define _UART1
#define _UART2
#define _UART3
// SPI
#define _SPI
// SYSTICK
#define _SYSTICK
// SSP
#define _SSP
#define _SSP0
#define _SSP1
// I2C
#define _I2C
#define _I2C0
#define _I2C1
#define _I2C2
// TIMER
#define _TIM
// WDT
#define _WDT
// GPDMA
#define _GPDMA
// DAC
#define _DAC
// ADC
#define _ADC
// PWM
#define _PWM
#define _PWM1
// RTC
#define _RTC
// I2S
#define _I2S
// USB device
#define _USBDEV
#define _USB_DMA
// QEI
#define _QEI
// MCPWM
#define _MCPWM
// CAN
#define _CAN
// RIT
#define _RIT
// EMAC
#define _EMAC

//=========================== prototypes ======================================

#endif /* _BOARD_INFO_H */
