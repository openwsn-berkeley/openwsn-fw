/**
\brief LPC17XX-specific definition of the "board" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#include "board.h"
#include "board_info.h"
#include "leds.h"
#include "bsp_timer.h"



//=========================== variables =======================================

//=========================== prototypes ======================================


//=========================== public ==========================================

void board_init() {

	leds_init();
	bsp_timer_init();
	
}

void board_sleep() {
	
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================


