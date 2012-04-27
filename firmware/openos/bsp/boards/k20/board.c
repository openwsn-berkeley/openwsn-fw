/**
\brief LPC17XX-specific definition of the "board" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#include "board.h"
#include "board_info.h"
#include "leds.h"
#include "led.h"
#include "bsp_timer.h"
#include "smc.h"
#include "mcg.h"
#include "sysinit.h"

extern int mcg_clk_hz;


//=========================== variables =======================================

//=========================== prototypes ======================================


//=========================== public ==========================================

void board_init() {
	uint8_t mcgmode=0;
	//enable all port clocks.
	SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK
			| SIM_SCGC5_PORTB_MASK
			| SIM_SCGC5_PORTC_MASK
			| SIM_SCGC5_PORTD_MASK
			| SIM_SCGC5_PORTE_MASK );


	/*Enable all operation modes because this is a write once register*/  
	SMC_PMPROT |= SMC_PMPROT_ALLS_MASK;

	

	mcgmode= what_mcg_mode();
	
	//ENABLE_GPIO_CLOCKS;

	llwu_init();
	debugpins_init();
	leds_init();
	bsp_timer_init();
	leds_all_off();

}

void board_sleep() {
	uint8_t op_mode;
	enter_lls();

	op_mode = what_mcg_mode();
	if(op_mode==PBE)
	{
		mcg_clk_hz = pbe_pee(CLK0_FREQ_HZ);
	}
	//enter_wait();
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================


