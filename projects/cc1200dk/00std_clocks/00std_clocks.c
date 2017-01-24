/** TO CHANGE this
\brief This is a standalone test program for the clocks on the MSP430F5438A of 
the TRXEB board.

Download the program to a TRXEB board, run it. It will configure the clock
sources of the TRXEB, and output its clocks on pins.

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
#include "msp430f5438a.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/

int main(void)
{
  WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT
  __bis_SR_register(SCG0);                  // allows to unset the DCO flag error    
 // __bis_SR_register(SCG1);                // this switches off the smclk
 
  // Initialize LFXT1
  P7SEL |= 0x03;                            // Select XT1
  UCSCTL6 &= ~(XT1OFF);                     // XT1 On
  UCSCTL6 |= XCAP_3;                        // Internal load cap
    // Loop until XT1 fault flag is cleared
  do
  {
    UCSCTL7 &= ~XT1LFOFFG;                  // Clear XT1 fault flags
  }while (UCSCTL7&XT1LFOFFG);               // Test XT1 fault flag
  
 // UCSCTL8 = 0x0007;
  P6DIR |= BIT1;                            // P1.0 output
  P11DIR |= 0x07;                           // ACLK, MCLK, SMCLK set out to pins
  P11SEL |= 0x07;                           // P11.0,1,2 for debugging purposes.
  P4DIR |= BIT0 | BIT1 | BIT2 | BIT3 ;
  
  UCSCTL0 = DCO3 /*| DCO1 *//*| MOD0 */;              //  ~22 MHz , DCO3+DCO1 and DCORSEL_&
  UCSCTL1 = DCORSEL_7 | DISMOD;     
  UCSCTL2 = 0;
  UCSCTL3 = 0;
  UCSCTL4 =  SELM_3 | SELS_3 ;
  
 
  do
  {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG );
                                       // Clear XT2,XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
  }while (SFRIFG1&OFIFG);

    
  while(1)
  {
    P4OUT ^= BIT0 | BIT1 | BIT2 | BIT3 ;
    P6OUT ^= BIT1;                          // Toggle P6.1
      __delay_cycles(22000000);             // Delay ~ 1s
  }
}

