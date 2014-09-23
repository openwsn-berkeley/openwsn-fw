/**
\brief This is a standalone test program for the LEDs and 32kHz crystal of the
       WSN430v14 board.

\note The term "crystal" is usually written "XTAL" by embedded geeks.
       
Download the program to a WSN430v14 board, run it, you should see the 3 LEDs
blinking in sequence with a 500ms period.

The digital outputs are:
   - P5.4: red LED
   - P5.5: green LED
   - P5.6: blue LED

The "inputs" are:
   - XIN, XOUT: 32768Hz crystal oscillator

The debug pins are:
   - P2.1 toggles when interrupt TIMERA0_VECTOR fires

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "msp430f1611.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/
int main(void)
{
   WDTCTL     =  WDTPW + WDTHOLD;                // disable watchdog timer
   
   DCOCTL     =  DCO0 | DCO1 | DCO2;             // MCLK at 8MHz
   BCSCTL1    =  RSEL0 | RSEL1 | RSEL2;          // MCLK at 8MHz
   
   P5DIR     |=  0x70;                           // P5DIR = 0bx111xxxx for LEDs
   P5OUT     |=  0x70;                           // P2OUT = 0bx111xxxx, all LEDs off
   
   P2DIR     |=  0x02;                           // P2DIR = 0bxxxxxx1x for debug

   TACCTL0    =  CCIE;                           // capture/compare interrupt enable
   TACCR0     =  16000;                          // 16000@32kHz ~ 500ms
   TACTL      =  MC_1+TASSEL_1;                  // up mode, using ACLK

   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

/**
\brief This function is called when the TimerA interrupt fires.
*/
#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR (void) {
   uint8_t leds_on;
   
   // get LED state
   leds_on    = (~P5OUT & 0x70) >> 4;
   
   // modify LED state
   if (leds_on==0) {                             // if no LEDs on, switch on one
      leds_on = 0x01;
   } else {
      leds_on += 1;
   }
   
   // apply updated LED state
   leds_on  <<=  4;                              // send back to position 4
   P5OUT     |=  (~leds_on & 0x70);              // switch on the leds marked '1' in leds_on
   P5OUT     &= ~( leds_on & 0x70);              // switch off the leds marked '0' in leds_on
   
   P2OUT     ^=  0x02;                           // toggle P2.1 for debug
}
