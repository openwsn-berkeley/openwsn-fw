/**
\brief ELDORADO-specific definition of the "board" bsp module.

\ authos Vitor Mangueira, Branko Kerkez <bkerkez@berkeley.edu>
*/
#include "eldorado.h"
#include "board.h"
// bsp modules
#include "leds.h"
#include "uart.h"
//#include "spi.h"
//#include "spi.h"
//#include "radio.h"
//#include "radiotimer.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {

  DisableWatchdog;
  //at the beginning, we source our uC off the internal clock (8MhZ), whihc is innacurate
  //once set up, we will use spi com. to the radio to use the 16MHz radio crystal as a time source
  ICGC1 = 0x28;
  ICGC2 = 0x61; //page 14-16 of data sheet N=16 R=2 ~17773714.3Mhz clock, 8886857.14 Mhz bus speed
 

   // initialize bsp modules
   leds_init();
   //uart_init();
   spi_init();
   radio_init();
   //radiotimer_init();
  
   
   __EI();//enable interrupts;
   
   
}

void board_sleep() {
  //poipoi __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

//=========================== private =========================================