/**
\brief This is a standalone test program for the button of the TelosB
board.

Download the program to a TelosB board, run it, and when you press the USER
button, the LEDs should shift circularly. Note that the RESET button is non
programmable, and always resets the MSP430.

The digital outputs are:
   - P5.4: red LED
   - P5.5: green LED
   - P5.6: blue LED
 
The digital inputs are:
   - P2.7: button
 
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2011
*/

#include "test_button.h"
#include "leds.h"
#include "button.h"

int main(void) {
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   
   DCOCTL  = DCO0 | DCO1 | DCO2;                 // MCLK at 8MHz
   BCSCTL1 = RSEL0 | RSEL1 | RSEL2;              // MCLK at 8MHz
   
   leds_init();
   button_init();
   
   __bis_SR_register(GIE+LPM4_bits);             // sleep
}

// Port2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port2_ISR (void) {
   P2IFG &= ~0x80;                               // clear interrupt flag
   leds_circular_shift();                        // wiggle LEDs
}
