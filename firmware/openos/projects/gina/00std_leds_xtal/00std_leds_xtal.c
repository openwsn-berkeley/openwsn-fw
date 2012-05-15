/**
\brief This is a standalone test program for the LEDs and 32kHz crystal of the
       GINA board.

\note The term "crystal" is usually written "XTAL" by embedded geeks.
       
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

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "msp430x26x.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/
int main(void)
{
   WDTCTL     =  WDTPW + WDTHOLD;                // disable watchdog timer
   BCSCTL1    =  CALBC1_16MHZ;                   // MCLK at 16MHz
   DCOCTL     =  CALDCO_16MHZ;
   
   P2DIR     |=  0x0F;                           // P2DIR = 0bxxxx1111 for LEDs
   P2OUT     &= ~0x0F;                           // P2OUT = 0bxxxx0000, all LEDs off
   
   P1DIR     |=  0x02;                           // P1DIR = 0bxxxxxx1x for debug
   P5DIR     |=  0x40;                           // P5.6 to ouput ACLK (for debug)
   P5SEL     |=  0x40;

   BCSCTL3   |=  LFXT1S_0;                       // ACLK sources from external 32kHz
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
   uint8_t temp_leds;
   
   // if no LEDs on, switch on first one
   if ((P2OUT & 0x0F)==0) {
      P2OUT  |=  0x01;
      return;
   }
   
   // get LED state
   temp_leds  =  P2OUT & 0x0F;                   // retrieve current status of LEDs
   temp_leds <<= 1;                              // shift by one position
   if ((temp_leds & 0x10)!=0) {
      temp_leds++;                               // handle overflow
   }
   P2OUT     |=  temp_leds;                      // switch on the leds marked '1' in temp_leds
   P2OUT     &= ~(~temp_leds & 0x0F);            // switch off the leds marked '0' in temp_leds
   
   P1OUT     ^=  0x02;                           // toggle P1.1 for debug
}
