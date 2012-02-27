/**
\brief GINA-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "board.h"
#include "leds.h"
#include "uart.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {
   // disable watchdog timer
   WDTCTL  = WDTPW + WDTHOLD;
   
   // setup clock speed
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at ~16MHz
   DCOCTL  = CALDCO_16MHZ;                       // MCLK at ~16MHz
   
   // initialize bsp modules
   leds_init();
   uart_init();
   
   // enable interrupts
   __bis_SR_register(GIE);
}

void board_sleep() {
   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

//=========================== private =========================================