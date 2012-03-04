/**
\brief Cross-platform declaration "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __BOARD_H
#define __BOARD_H

#include "board_info.h"

//=========================== define ==========================================

//poipoipoi
#define CAPTURE_TIME() 

enum radio_antennaselection_enum {
   RADIO_UFL_ANTENNA              = 0x06, ///< Use the antenna connected by U.FL.
   RADIO_CHIP_ANTENNA             = 0x05, ///< Use the on-board chip antenna.
};

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void board_init();
void board_sleep();

#endif
