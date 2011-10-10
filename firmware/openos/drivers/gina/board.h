/**
\brief GINA's board service package

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#ifndef __BOARD_H
#define __BOARD_H

#include "openwsn.h"

//=========================== define ==========================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "GINA";
static const uint8_t infouCName[]    = "MSP430f2618";
static const uint8_t infoRadioName[] = "AT86RF231";

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void board_init();

#endif
