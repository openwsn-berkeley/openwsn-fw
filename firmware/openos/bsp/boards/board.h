/**
\brief Cross-platform declaration "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __BOARD_H
#define __BOARD_H

#include "board_info.h"

//=========================== define ==========================================

#define DO_NOT_KICK_SCHEDULER     0
#define KICK_SCHEDULER            1

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void board_init();
void board_sleep();

#endif
