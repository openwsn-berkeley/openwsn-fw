/**
\brief Cross-platform declaration "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __BOARD_H
#define __BOARD_H

#include "board_info.h"

//=========================== define ==========================================

typedef enum {
   DO_NOT_KICK_SCHEDULER,
   KICK_SCHEDULER,
} kick_scheduler_t;

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void board_init();
void board_sleep();

#endif
