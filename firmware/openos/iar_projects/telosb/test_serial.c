/**
\brief This is a standalone test program for serial communication between the
       TelosB and a computer.

The digital UART interface is:
   - P3.7: A1_SOMI_RX
   - P3.8: A1_SOMI_TX

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "gina.h"
#include "leds.h"

void main(void)
{
  //configuring
  gina_init();
    
  P3SEL     = 0xC0;                              // P3.6,7 = USCI_A1 TXD/RXD
  /*
  //9600 baud, clocked from 32kHz ACLK
  UCA1CTL1 |= UCSSEL_1;                          // CLK = ACLK
  UCA1BR0   = 0x03;                              // 9600 baud if ACLK@32kHz
  UCA1BR1   = 0x00;
  UCA1MCTL  = UCBRS_3;                           // Modulation UCBRSx = 3
  */
  
  //115200 baud, clocked from 16MHz SMCLK
  UCA1CTL1 |= UCSSEL_2;                          // CLK = SMCL
  UCA1BR0   = 0x8a;                              // 115200 baud if SMCLK@16MHz
  UCA1BR1   = 0x00;
  UCA1MCTL  = UCBRS_7;                           // Modulation UCBRSx = 7
  
  /*
  //256000 baud, clocked from 16MHz SMCLK
  UCA1CTL1 |= UCSSEL_2;                          // CLK = SMCL
  UCA1BR0   = 0x3E;                              // 256000 baud if SMCLK@16MHz
  UCA1BR1   = 0x00;
  UCA1MCTL  = UCBRS_4;                           // Modulation UCBRSx = 4
  */
  
  UCA1CTL1 &= ~UCSWRST;                          // Initialize USCI state machine
  UC1IFG   &= ~(UCA1TXIFG | UCA1RXIFG);          // clear possible pending interrupts
  UC1IE    |=  (UCA1RXIE  | UCA1TXIE);           // Enable USCI_A1 TX & RX interrupt
  
  __bis_SR_register(LPM4_bits + GIE);            // sleep, leave interrupts on
}

#pragma vector=USCIAB1RX_VECTOR
__interrupt void USCI1RX_ISR(void)
{
  UCA1TXBUF = UCA1RXBUF;                        // TX -> RXed character
  leds_circular_shift();
}

#pragma vector=USCIAB1TX_VECTOR
__interrupt void USCI1TX_ISR(void)
{
  UC1IFG &= ~UCA1TXIFG;
  leds_circular_shift();
}
