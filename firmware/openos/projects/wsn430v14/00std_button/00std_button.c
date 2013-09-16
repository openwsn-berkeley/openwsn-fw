/**
\brief This is a standalone test program for the button of the TelosB
       board.

Download the program to a TelosB board, run it, and when you press the USER
button, the red LED with toggle. Note that the RESET button is non 
programmable, and always resets the MSP430.

\note To "toggle" a pin means that, when it's off, it turns on, and when it's
      on, it turns off.

The digital outputs are:
   - P5.4: red LED
 
The digital inputs are:
   - P2.7: button
 
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "msp430f1611.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/
int main(void) {
   WDTCTL     =  WDTPW + WDTHOLD;                // disable watchdog timer
   
   DCOCTL     =  DCO0 | DCO1 | DCO2;             // MCLK at 8MHz
   BCSCTL1    =  RSEL0 | RSEL1 | RSEL2;          // MCLK at 8MHz
   
   P5DIR     |=  0x70;                           // P5DIR = 0bx111xxxx for LEDs
   P5OUT     |=  0x70;                           // P2OUT = 0bx111xxxx, all LEDs off
   
   // button connected to P2.7, i.e. configuration 0x80 in P2DIR/P2OUT/P2IN registers
   P2DIR     &= ~0x80;                           // input direction
   P2OUT     |=  0x80;                           // put pin high as pushing button brings low
   P2IES     |=  0x80;                           // interrup when transition is high-to-low
   P2IE      |=  0x80;                           // enable interrupts
   
   __bis_SR_register(GIE+LPM4_bits);             // sleep
}

/**
\brief This function is called when the Port2 interrupt fires.
*/
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR (void) {
   P2IFG &= ~0x80;                               // clear the interrupt flag
   P5OUT ^=  0x10;                               // toggle LED
}
