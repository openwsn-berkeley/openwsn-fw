/**
 \brief LPC1769-specific board information bsp module.

 This module simply defines some strings describing the board, which CoAP uses
 to return the board's description.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#ifndef _BOARD_INFO_H
#define _BOARD_INFO_H

#include "stdint.h"
#include "LPC17xx.h"
#include "lpc_types.h"

//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "xpressohack";
static const uint8_t infouCName[]    = "NXP LPC1769";
static const uint8_t infoRadioName[] = "AT86RF231";

//=========================== defines =========================================

//#define SPI_IN_INTERRUPT_MODE
//#define SPI_IN_RTOS_MODE

// [P2.8] SLP_TR
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()   LPC_GPIO2->FIOSET |=  1<<8;
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()    LPC_GPIO2->FIOCLR |=  1<<8;

// [P2.4] radio RSTn
#define PORT_PIN_RADIO_RESET_HIGH()         LPC_GPIO2->FIOSET |=  1<<4;
#define PORT_PIN_RADIO_RESET_LOW()          LPC_GPIO2->FIOCLR |=  1<<4;

//isr radio is GPIO 2.5

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
