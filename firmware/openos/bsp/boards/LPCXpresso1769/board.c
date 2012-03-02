/**
\brief LPC17XX-specific definition of the "board" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/

#include "LPC17xx.h"
#include <board.h>
#include <leds.h>
#include "uart.h"
#include "spi.h"
#include "radio.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {

	//call system_init from system_LPC17xx.c
	//SystemInit();


   // initialize bsp modules
   leds_init();
   uart_init();
   spi_init();
  // radio_init();
}

void board_sleep() {
   //__bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
	__WFI();
}

//=========================== private =========================================
