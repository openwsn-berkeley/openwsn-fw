/**
\brief This is a standalone test program for the clocks on the WSN430v14
       board.

Download the program to a WSN430v14 board, run it. It will configure the clock
sources of the WSN430v14, and output its clocks on pins.

The following clock sources are used:
- the  8MHz crystal on the WSN40v14 is connected to the XT2 clock source
- the 32kHz crystal on the WSN40v14 is connected to the LFXT1

From those two, the following clock signals are created:
-  MCLK is set to XT2,   i.e.  8MHz
- SMCLK is set to XT2/8, i.e.  1MHz
-  ACLK is set to LFXT1, i.e. 32kHz

The digital outputs are:
- P5.4: MCLK  (red LED),   expected period   125ns
- P5.5: SMCLK (green LED), expected period  1000ns
- P5.6: ACLK  (blue LED),  expected period 30517ns

Note that these pins are connected to the LEDs, so they will blink (so fast
they will appear simply on to your slow eyes). Use a scope os logic analyzer
to see the clock signals and measure their frequency.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "msp430f1611.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/
int main(void) {
   uint8_t delay;
   
   WDTCTL     = WDTPW + WDTHOLD;                 // disable watchdog timer
   
   DCOCTL     = 0;                               // we are not using the DCO
   BCSCTL1    = 0;                               // we are not using the DCO
   BCSCTL2    = SELM_2 | (SELS | DIVS_3) ;       // MCLK=XT2, SMCLK=XT2/8
   
   // the MSP detected that the crystal is not running (it's normal, it is
   // starting). It set the OFIFG flag, causing the MSP430 to switch back to
   // the DC0. By software, we need to clear that flag, causing the MSP430 to
   // switch back to using the XT2 as a clocking source, but verify that it
   // stays cleared. This is explained in detail in Section 4.2.6 "Basic Clock
   // Module Fail-Safe Operation" of slau049f, pdf page 119.
   
   do {
      IFG1   &= ~OFIFG;                          // clear OSCFault flag
      for (delay=0;delay<0xff;delay++) {         // busy wait for at least 50us
          __no_operation();
      }
   } while ((IFG1 & OFIFG) != 0);                // repeat until OSCFault flag stays cleared
   
   // output signals on pins
   P5OUT     |= 0x70;                            // P5OUT = 0xx111xxxx -> P5.4-6 output
   P5SEL     |= 0x70;                            // P5SEL = 0xx111xxxx -> P5.4-6 in clock output mode
   
   while(1);
}

