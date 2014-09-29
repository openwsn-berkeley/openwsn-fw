/**
\brief This is a standalone test program for serial communication between the GINA2.2b/c
       and a computer using the breakout board.

The digital UART interface is:
   - P3.7: A1_SOMI_RX
   - P3.8: A1_SOMI_TX

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

//#define BAUDRATE_256000 // uncomment this to communicate at 245000baud
//#define BAUDRATE_115200 // uncomment this to communicate at 115200baud

#include "msp430x26x.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/
void main(void)
{
   WDTCTL     =  WDTPW + WDTHOLD;                // disable watchdog timer
   BCSCTL1    =  CALBC1_16MHZ;                   // MCLK at 16MHz
   DCOCTL     =  CALDCO_16MHZ;
   
   P3SEL      =  0xc0;                           // P3.6,7 = USCI_A1 TXD/RXD
   
   P2DIR     |=  0x0F;                           // P2DIR = 0bxxxx1111 for LEDs
   P2OUT     &= ~0x0F;                           // P2OUT = 0bxxxx0000, all LEDs off
   
#ifdef BAUDRATE_256000
   //256000 baud, clocked from 16MHz SMCLK
   UCA1CTL1  |=  UCSSEL_2;                       // CLK = SMCL
   UCA1BR0    =  0x3E;                           // 256000 baud if SMCLK@16MHz
   UCA1BR1    =  0x00;
   UCA1MCTL   =  UCBRS_4;                        // Modulation UCBRSx = 4
#elif  BAUDRATE_115200
   //115200 baud, clocked from 16MHz SMCLK
   UCA1CTL1  |=  UCSSEL_2;                       // CLK = SMCL
   UCA1BR0    =  0x8a;                           // 115200 baud if SMCLK@16MHz
   UCA1BR1    =  0x00;
   UCA1MCTL   =  UCBRS_7;                        // Modulation UCBRSx = 7
#else 
   //9600 baud, clocked from 32kHz ACLK
   UCA1CTL1  |=  UCSSEL_1;                       // CLK = ACLK
   UCA1BR0    =  0x03;                           // 9600 baud if ACLK@32kHz
   UCA1BR1    =  0x00;
   UCA1MCTL   =  UCBRS_3;                        // Modulation UCBRSx = 3
#endif
   
   UCA1CTL1  &= ~UCSWRST;                        // Initialize USCI state machine
   UC1IFG    &= ~(UCA1TXIFG | UCA1RXIFG);        // clear possible pending interrupts
   UC1IE     |=  (UCA1RXIE  | UCA1TXIE);         // Enable USCI_A1 TX & RX interrupt
   
   __bis_SR_register(LPM4_bits + GIE);           // sleep, leave interrupts on
}

/**
\brief This function is called when the the UART module has received a byte.
*/
#pragma vector = USCIAB1RX_VECTOR
__interrupt void USCIAB1RX_ISR (void)
{
   UCA1TXBUF  =  UCA1RXBUF;                      // TX -> RXed character
   P2OUT     ^=  0x02;                           // toggle green LED
}

/**
\brief This function is called when the the UART module ready to send next byte.
*/
#pragma vector = USCIAB1TX_VECTOR
__interrupt void USCIAB1TX_ISR (void)
{
   UC1IFG    &= ~UCA1TXIFG;                      // clear interrupt flag
   P2OUT     ^=  0x08;                           // toggle red LED
}
