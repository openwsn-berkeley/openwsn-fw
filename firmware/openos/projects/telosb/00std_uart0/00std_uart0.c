/**
\brief This is a standalone test program for serial communication over both
       UART1 and UART0 between the TelosB and a computer.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "msp430f1611.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/
void main(void)
{
   WDTCTL     =  WDTPW + WDTHOLD;                // disable watchdog timer
   
   DCOCTL     =  DCO0 | DCO1 | DCO2;             // MCLK at ~8MHz
   BCSCTL1    =  RSEL0 | RSEL1 | RSEL2;          // MCLK at ~8MHz
                                                 // by default, ACLK from 32kHz XTAL which is running
   P5DIR     |=  0x70;                           // P5DIR = 0bx111xxxx for LEDs
   P5OUT     |=  0x70;                           // P2OUT = 0bx111xxxx, all LEDs off
   
   //=== UART0
   
   // pins
   P3SEL     |=  0x30;                           // P3SEL = 0bxx11xxxx (P3.4,5) UART0TX/RX
   
   //UART 0 9600 baud, clocked from 32kHz ACLK
   ME1       |=  UTXE0 + URXE0;                  // enable UART0 TX/RX
   U0CTL     |=  CHAR;                           // 8-bit character
   U0TCTL    |=  SSEL0;                          // clocking from ACLK
   U0BR0      =  0x03;                           // 32768/9600 = 3.41
   U0BR1      =  0x00;                           //
   U0MCTL     =  0x4A;                           // modulation
   U0CTL     &= ~SWRST;                          // clear UART0 reset bit
   IE1       |=  URXIE0;                         // enable UART0 RX interrupt
   
   //=== UART1
   
   // pins
   P3SEL     |=  0xC0;                           // P3SEL = 0b11xxxxxx (P3.6,7) UART1TX/RX
   
   //9600 baud, clocked from 32kHz ACLK
   ME2       |=  UTXE1 + URXE1;                  // enable UART1 TX/RX
   U1CTL     |=  CHAR;                           // 8-bit character
   U1TCTL    |=  SSEL0;                          // clocking from ACLK
   U1BR0      =  0x03;                           // 32768/9600 = 3.41
   U1BR1      =  0x00;                           //
   U1MCTL     =  0x4A;                           // modulation
   U1CTL     &= ~SWRST;                          // clear UART1 reset bit
   IE2       |=  URXIE1;                         // enable UART1 RX interrupt
   
   __bis_SR_register(LPM3_bits + GIE);           // sleep, leave interrupts and ACLK on
}

/**
\brief Called when receiving byte from UART0 (extension headers)
*/
#pragma vector = USART0RX_VECTOR
__interrupt void USART0RX_ISR (void)
{
   //U0TXBUF   =  U0RXBUF;                        // echo char over extension header
   U1TXBUF   =  U0RXBUF;                        // send char over USB
   P5OUT    ^=  0x10;                           // toggle LED
}

/**
\brief Called when receiving byte from UART0 (USB headers)
*/
#pragma vector = USART1RX_VECTOR
__interrupt void USART1RX_ISR (void)
{
   U1TXBUF   =  U1RXBUF;                        // echo char over USB
   U0TXBUF   =  U1RXBUF;                        // send char over extension header
   P5OUT    ^=  0x10;                           // toggle LED
}