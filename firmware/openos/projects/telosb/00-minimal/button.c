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

#include "msp430f1611.h"
#include "stdint.h"

int main(void) {
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   
   DCOCTL  = DCO0 | DCO1 | DCO2;                 // MCLK at 8MHz
   BCSCTL1 = RSEL0 | RSEL1 | RSEL2;              // MCLK at 8MHz
   
   P5DIR  |=  0x70;                              // P5DIR = 0bx111xxxx for LEDs
   P5OUT  |=  0x70;                              // P2OUT = 0bx111xxxx, all LEDs off
   
   // button connected to P2.7, i.e. configuration 0x80 in P2XX register
   P2DIR  &= ~0x80;                              // input direction
   
   P2OUT  |=  0x80;                              // put pin high as pushing button brings low
   P2IES  |=  0x80;                              // interrup when transition is high-to-low
   P2IE   |=  0x80;                              // enable interrupts
   
   __bis_SR_register(GIE+LPM4_bits);             // sleep
}

// Port2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port2_ISR (void) {
   uint8_t leds_on;
   
   P2IFG &= ~0x80;                               // clear interrupt flag
   
   // get LED state
   leds_on  = (~P5OUT & 0x70) >> 4;
   
   // modify LED state
   if (leds_on==0) {                             // if no LEDs on, switch on one
      leds_on = 0x01;
   } else {
      leds_on += 1;
   }
   // apply updated LED state
   leds_on <<= 4;                                // send back to position 4
   P5OUT |=  (~leds_on & 0x70);                  // switch on the leds marked '1' in leds_on
   P5OUT &= ~( leds_on & 0x70);                  // switch off the leds marked '0' in leds_on
}
