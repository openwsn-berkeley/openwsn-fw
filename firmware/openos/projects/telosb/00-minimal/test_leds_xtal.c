/**
\brief This is a standalone test program for the LEDs of the TelosB
       board.
       
Download the program to a TelosB board, run it, you should see
the 3 LEDs blinking in sequence with a 500ms period.

The digital outputs are:
   - P5.4: red LED
   - P5.5: green LED
   - P5.6: blue LED

The "inputs" are:
   - XIN, XOUT: 32768Hz crystal oscillator

The debug pins are:
   - P6.6 toggles when interrupt TIMERA0_VECTOR fires

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2011
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "leds.h"

int main(void)
{
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   
   DCOCTL  = DCO0 | DCO1 | DCO2;                 // MCLK at 8MHz
   BCSCTL1 = RSEL0 | RSEL1 | RSEL2;              // MCLK at 8MHz
   
   leds_init();
   
   P6DIR  |=  0x40;                              // P4DIR = 0bx1xxxxxx for debug

   TACCTL0  = CCIE;                              // capture/compare interrupt enable
   TACCR0   = 16000;                             // 16000@32kHz ~ 500ms
   TACTL    = MC_1+TASSEL_1;                     // up mode, using ACLK

   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

// TimerA interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A_ISR (void) {
   P6OUT  ^=  0x40;                              // toggle P6.6 for debug
   leds_increment();
}
