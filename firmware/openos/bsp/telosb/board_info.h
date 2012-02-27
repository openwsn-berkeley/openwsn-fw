/**
\brief TelosB-specific board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"

//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "TelosB";
static const uint8_t infouCName[]    = "MSP430f1611";
static const uint8_t infoRadioName[] = "CC2420";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================