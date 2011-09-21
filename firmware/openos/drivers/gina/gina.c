/**
\brief GINA's board service package

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "gina.h"
#include "leds.h"

#ifdef ISR_I2C
#include "i2c.h"
#endif

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void gina_init() {
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;

   P1OUT  &= ~0x06;                              // P1.1,2 low
   P1DIR  |=  0x06;                              // P1.1,2 as output
   
   leds_init();
#ifdef ISR_I2C
   i2c_init();
#endif
   __bis_SR_register(GIE);                       // set 'general interrupt enable' bit
}

//=========================== private =========================================