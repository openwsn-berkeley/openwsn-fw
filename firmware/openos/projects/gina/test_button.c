/**
\brief This is a standalone test program for the button of the GINA2.2b/c
board.

Download the program to a GINA board, run it, and when you press the AUX
button, the LEDs should shift circularly. Note that the RESET button is non
programmable, and always resets the MSP430.

The digital outputs are:
  - P2.0: red LED
  - P2.1: green LED
  - P2.2: blue LED
  - P2.3: red LED
 
 The digital inputs are:
   - P2.7: button
 
 \author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
 */

#include "board.h"
#include "openwsn.h"
#include "test_button.h"
#include "leds.h"
#include "button.h"
#include "scheduler.h"

int main(void) {
   gina_init();
   scheduler_init();
   leds_init();
   button_init();
   
   scheduler_start();
   return 0;
}

void isr_button() {
   leds_circular_shift();                        // circular-shift the LEDs
}
