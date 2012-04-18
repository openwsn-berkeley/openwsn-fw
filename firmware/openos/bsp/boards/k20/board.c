/**
\brief LPC17XX-specific definition of the "board" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#include "board.h"
#include "board_info.h"
#include "leds.h"
#include "bsp_timer.h"
#include "smc.h"



//=========================== variables =======================================

//=========================== prototypes ======================================


//=========================== public ==========================================

void board_init() {
    //enable all port clocks.
//	SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK
//	| SIM_SCGC5_PORTB_MASK
//	| SIM_SCGC5_PORTC_MASK
//	| SIM_SCGC5_PORTD_MASK
//	| SIM_SCGC5_PORTE_MASK );

	
	leds_init();
	bsp_timer_init();
	
}

void board_sleep() {
	enter_wait();
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================


