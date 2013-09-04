/**
\brief This is a standalone test program for the clocks on the TelosB
       board.

Download the program to a TelosB board, run it. It will output its
clocks on the pins below. Note that these pins are connected to the
LEDs, so they will blink (so fast they will appear simply on to your slow eyes).
Use a scope probe to see the clock signals and measure their frequency.

The digital outputs are:
   - P5.4: MCLK  (red LED or pad 48 on the back)
   - P5.5: SMCLK (green LED or pad 49 on the back)
   - P5.6: ACLK  (blue LED)

We measure an DCO frequency of 4.8MHz. Very low, indeed.
 
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "msp430f1611.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/
int main(void) {
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   
   DCOCTL  = DCO0 | DCO1 | DCO2;                 // MCLK at 8MHz
   BCSCTL1 = RSEL0 | RSEL1 | RSEL2;              // MCLK at 8MHz
                                                 // by default, ACLK from 32kHz XTAL which is running

   P5OUT  |= 0x70;                               // P5OUT = 0xx111xxxx -> P5.4-6 output
   P5SEL  |= 0x70;                               // P5SEL = 0xx111xxxx -> P5.4-6 in clock output mode
   
   while(1);                                     // don't sleep so all clocks run
}