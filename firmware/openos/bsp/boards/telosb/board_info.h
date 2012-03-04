/**
\brief TelosB-specific board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
#include "msp430f1611.h"

//=========================== define ==========================================

// on TelosB, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()                  CACTL1 |= CAIFG
#define SCHEDULER_ENABLE_INTERRUPT()        CACTL1  = CAIE

// [P4.5] radio VREG
#define PORT_PIN_RADIO_VREG_HIGH()          P4OUT |=  0x20;
#define PORT_PIN_RADIO_VREG_LOW()           P4OUT &= ~0x20;
// [P4.6] radio RESET
#define PORT_PIN_RADIO_RESET_HIGH()         P4OUT |=  0x40;
#define PORT_PIN_RADIO_RESET_LOW()          P4OUT &= ~0x40;  

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "TelosB";
static const uint8_t infouCName[]           = "MSP430f1611";
static const uint8_t infoRadioName[]        = "CC2420";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================