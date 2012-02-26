/**
\brief TelosB-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "board.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "radio.h"
#include "radiotimer.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {
   // disable watchdog timer
   WDTCTL     =  WDTPW + WDTHOLD;
   
   // setup clock speed
   DCOCTL     =  DCO0 | DCO1 | DCO2;             // MCLK at ~8MHz
   BCSCTL1    =  RSEL0 | RSEL1 | RSEL2;          // MCLK at ~8MHz
                                                 // by default, ACLK from 32kHz XTAL which is running
   
   // initialize bsp modules
   leds_init();
   uart_init();
   //spi_init();
   //radio_init();
   //radiotimer_init();
   
   __bis_SR_register(GIE);
}

void board_sleep() {
   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

//=========================== private =========================================