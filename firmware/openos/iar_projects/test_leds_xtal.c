/**
\brief This is a standalone test program for the LEDs of the GINA2.2b/c
       board.
       
Download the program to a GINA board, run it, you should see
the 4 LEDs blinking in sequence with a 500ms period.

The digital outputs are:
   - P2.0: red LED
   - P2.1: green LED
   - P2.2: blue LED
   - P2.3: red LED

The "inputs" are:
   - XIN, XOUT: 32768Hz crystal oscillator

The debug pins are:
   - P5.6 output the ACLK (should be 32768Hz)
   - P1.1 toggles when interrupt TIMERA0_VECTOR fires

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "leds.h"

int main(void)
{
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;
   
   leds_init();
   
   P1DIR  |=  0x02;                              // P1DIR = 0bxxxxxx1x for debug
   P5DIR  |=  0x40;                              // P5.6 to ouput ACLK (for debug)
   P5SEL  |=  0x40;

   BCSCTL3 |= LFXT1S_0;                          // ACLK sources from external 32kHz
   TACCTL0  = CCIE;                              // capture/compare interrupt enable
   TACCR0   = 16000;                             // 16000@32kHz ~ 500ms
   TACTL    = MC_1+TASSEL_1;                     // up mode, using ACLK

   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

// TimerA interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A_ISR (void) {
   P1OUT  ^=  0x02;                              // toggle P1.1 for debug
   leds_circular_shift();
}
