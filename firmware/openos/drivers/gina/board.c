/**
\brief GINA's board service package

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "board.h"
#include "leds.h"
#include "msp430x26x.h"
#include "timers.h"
#include "ieee154etimer.h"
#include "openserial.h"
#include "radio.h"
#include "i2c.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {
   // disable watchdog timer
   WDTCTL  = WDTPW + WDTHOLD;
   
   // clock MSP at 16MHz
   BCSCTL1 = CALBC1_16MHZ;
   DCOCTL  = CALDCO_16MHZ;
   
   // low-level drivers
   leds_init();
   i2c_init();
   timer_init();
   ieee154etimer_init();
   openserial_init();
   
   // high-level drivers
   radio_init();
   
   // set 'general interrupt enable' bit
   __bis_SR_register(GIE);
}

//=========================== private =========================================