/**
\brief This is a standalone test program for the timer of the GINA2.2b/c
       board.

Download the program to a GINA board, run it.

The "inputs" are:
   - XIN, XOUT: 32768Hz crystal oscillator

The debug pins are:
   - P5.6 output the ACLK (should be 32768Hz)
   - P1.1 toggles when interrupt TIMERA0_VECTOR fires
   - P1.2 toggles when interrupt TIMERA0_VECTOR fires

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "msp430x26x.h"
#include "stdint.h"

#define TIMERA0_PERIOD 0x0180
#define TIMERA1_PERIOD 0x0100
#define TIMERA2_PERIOD 0x0120

#define TIMERB0_PERIOD 0x0180
#define TIMERB1_PERIOD 0x0100
#define TIMERB2_PERIOD 0x0120
#define TIMERB3_PERIOD 0x0201
#define TIMERB4_PERIOD 0x0310
#define TIMERB5_PERIOD 0x0538
#define TIMERB6_PERIOD 0x0612

int main(void) {
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;
   
   P1DIR  |=  0x06;                              // P1DIR = 0bxxxxx11x for debug
   P5DIR  |=  0x40;                              // P5.6 to ouput ACLK (for debug)
   P5SEL  |=  0x40;
   
   //ACLK
   BCSCTL3 |= LFXT1S_0;                          // ACLK sources from external 32kHz
   
   //TimerA
   //--set CCRAx registers
   TACCTL0  = CCIE;                              // capture/compare 0 interrupt enable
   TACCR0   = TIMERA0_PERIOD;
   TACCTL1  = CCIE;                              // capture/compare 1 interrupt enable
   TACCR1   = TIMERA1_PERIOD;
   TACCTL2  = CCIE;                              // capture/compare 2 interrupt enable
   TACCR2   = TIMERA2_PERIOD;
   //--start timer on 32kHz ACLK
   TACTL    = MC_2+TASSEL_1;                     // continuous mode, using ACLK
   
   //TimerB
   //--set CCRBx registers
   TBCCTL0 |= CCIE;                              // capture/compare 0 interrupt enable
   TBCCR0   = TIMERB0_PERIOD;
   TBCCTL1  = CCIE;                              // capture/compare 1 interrupt enable
   TBCCR1   = TIMERB1_PERIOD;
   TBCCTL2  = CCIE;                              // capture/compare 2 interrupt enable
   TBCCR2   = TIMERB2_PERIOD;
   TBCCTL3  = CCIE;                              // capture/compare 3 interrupt enable
   TBCCR3   = TIMERB3_PERIOD;
   TBCCTL4  = CCIE;                              // capture/compare 4 interrupt enable
   TBCCR4   = TIMERB4_PERIOD;
   TBCCTL5  = CCIE;                              // capture/compare 5 interrupt enable
   TBCCR5   = TIMERB5_PERIOD;
   TBCCTL6  = CCIE;                              // capture/compare 6 interrupt enable
   TBCCR6   = TIMERB6_PERIOD;
   //--start timer on 32kHz ACLK
   TBCTL    = MC_2+TASSEL_1;                     // continuous mode, using ACLK

   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
   return 0;
}

// TimerA CCR0 interrupt service routine
#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR (void) {
   TACCR0 += TIMERA0_PERIOD;
   __no_operation();
}

// TimerA CCR1-2 interrupt service routine
#pragma vector = TIMERA1_VECTOR
__interrupt void TIMERA1_ISR (void) {
   uint16_t taiv_temp = TAIV;                    // necessary because accessing TAIV resets it
   switch (taiv_temp) {
   case 0x0002:
      TACCR1 += TIMERA1_PERIOD;
      P1OUT   ^=  0x04;                          // toggle P1.2 for debug
      break;
   case 0x0004:
      TACCR2 += TIMERA2_PERIOD;
      P1OUT   ^=  0x02;                          // toggle P1.1 for debug
      break;
   default:
      while(1);                                  // this should not happen
   }
}

// TimerB CCR0 interrupt service routine
#pragma vector = TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void) {
   TBCCR0 += TIMERB0_PERIOD;
   __no_operation();
}

// TimerB CCR1-6 interrupt service routine
#pragma vector = TIMERB1_VECTOR
__interrupt void TIMERB1_ISR (void) {
   uint16_t tbiv_temp = TBIV;                    // necessary because accessing TBIV resets it
   switch (tbiv_temp) {
   case 0x0002:
      TBCCR1 += TIMERB1_PERIOD;
      P1OUT   ^=  0x04;                          // toggle P1.2 for debug
      break;
   case 0x0004:
      TBCCR2 += TIMERB2_PERIOD;
      P1OUT   ^=  0x02;                          // toggle P1.1 for debug
      break;
   case 0x0006:
      TBCCR3 += TIMERB3_PERIOD;
      break;
   case 0x0008:
      TBCCR4 += TIMERB4_PERIOD;
      break;
   case 0x000A:
      TBCCR5 += TIMERB5_PERIOD;
      break;
   case 0x000C:
      TBCCR6 += TIMERB6_PERIOD;
      break;
   default:
      while(1);                                  // this should not happen
   }
}