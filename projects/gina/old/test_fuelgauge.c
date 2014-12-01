/**
\brief This is a standalone test program for the fuelgauge on the daughter card of
       the GINA2.2b/c.

The digital connection is done through two-wire I2C serial bus: TBC

The digital input is: TBC

The debug pins are: TBC

Speed: one measurement takes TBC

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, August 2010
*/

//!!!!!!!!!!THIS CODE IS STILL UNDER DVELOPMENT. DO NOT USE.

#include "fuelgauge.h"

void init ( void ) {
   WDTCTL = WDTPW + WDTHOLD;

   if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) {  
      while(1);                                    // If calibration constants erased
   }                                              // do not load, trap CPU!!
   BCSCTL1 = CALBC1_16MHZ;                        // Set DCO to 16MHz
   DCOCTL = CALDCO_16MHZ;                         // Set DCO to 16MHz
   BCSCTL3 = LFXT1S_2;                            // Set ACLK = VLOCLK (12 kHz)

   P1DIR = 0;
   P1IFG = 0;

   P2DIR = 0xFF;                                  // Set P2.0 to output direction
   P2OUT = 0;

   _BIS_SR(GIE);                                  // enable interrupts
}

int main( void ) {
   unsigned int i = 0;
   static int voltage;

   init();
   fuelgauge_init();
   fuelgauge_test();

   while(1) {
      voltage = fuelgauge_read(FUELGAUGE_REG_VOLT);
      P2OUT = ((voltage - 2500) >> 3);
      //voltage = fuelgauge_read(FUELGAUGE_REG_AI);
      //P2OUT = ((((-voltage) >> 1) - 16) << 4);

      while (--i);
   }
}

// USCI_B0 Data ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void) {
   isr_i2c_rx();
}
