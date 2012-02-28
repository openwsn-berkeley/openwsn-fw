/**
\brief LPC1769-specific board information bsp module.

This module simply defines some strings describing the board, which CoAP uses
to return the board's description.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/


#include "stdint.h"
//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "LPCXpresso1769";
static const uint8_t infouCName[]    = "NXP LPC1769";
static const uint8_t infoRadioName[] = "none";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================
