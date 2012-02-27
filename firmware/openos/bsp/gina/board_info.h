/**
\brief GINA-specific board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"

//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "GINA";
static const uint8_t infouCName[]    = "MSP430f2618";
static const uint8_t infoRadioName[] = "AT86RF231";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================