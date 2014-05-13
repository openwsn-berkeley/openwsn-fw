/**
\brief This is a standalone test program for serial communication between the
   WSN430 and a computer.

The MSP430 chip speaks 2-wire UART, i.e. two pins are used: one for sending
bytes (UART0TX), one for receiving bytes (UART0RX). When you plug the WSN430
into your computer, it appears as a serial COM port. That is, when you
read/write to that port, the bytes end up on the UART0RX/UART0TX pins.

Connect your WSN430 board to your computer, download this application to your
WSN430 board and run it. On your computer, open a PuTTY client on the 
COM port of your board, and type characters into it.

Each time you type a character, you should see:
- the character prints on your terminal, as it is sent back on the TX line.
- the red LED toggles

Uncomment the BAUDRATE_115200 line below to switch from 9600baud to 115200baud.

The digital UART interface is:
   - P3.4: UTXD0
   - P3.5: URXD0

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

//#define BAUDRATE_115200 // uncomment this to communicate at 115200baud

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
   P5DIR     |=  0x70;                           // P5DIR = 0bx111xxxx, all LEDs output
   P5OUT     |=  0x70;                           // P5OUT = 0bx111xxxx, all LEDs off
   
   P3SEL      =  0x30;                           // P3.4,5 = UART0TX/RX
  
#ifdef BAUDRATE_115200
   //115200 baud, clocked from 4.8MHz SMCLK
   ME1       |=  UTXE0 + URXE0;                  // enable UART0 TX/RX
   U0CTL     |=  CHAR;                           // 8-bit character
   U0TCTL    |=  SSEL1;                          // clocking from SMCLK
   U0BR0      =  41;                             // 4.8MHz/115200 - 41.66
   U0BR1      =  0x00;                           //
   U0MCTL     =  0x4A;                           // modulation
   U0CTL     &= ~SWRST;                          // clear UART1 reset bit
   IE1       |=  URXIE0;                         // enable UART1 RX interrupt
   
   __bis_SR_register(LPM0_bits + GIE);           // sleep, leave interrupts on
#else
   //9600 baud, clocked from 32kHz ACLK
   ME1       |=  UTXE0 + URXE0;                  // enable UART1 TX/RX
   U0CTL     |=  CHAR;                           // 8-bit character
   U0TCTL    |=  SSEL0;                          // clocking from ACLK
   U0BR0      =  0x03;                           // 32768/9600 = 3.41
   U0BR1      =  0x00;                           //
   U0MCTL     =  0x4A;                           // modulation
   U0CTL     &= ~SWRST;                          // clear UART0 reset bit
   IE1       |=  URXIE0;                         // enable UART0 RX interrupt
   
   __bis_SR_register(LPM3_bits + GIE);           // sleep, leave interrupts and ACLK on
#endif  
}

/**
\brief This function is called when the the UART module has received a byte.
*/
#pragma vector = USART0RX_VECTOR
__interrupt void USART0RX_ISR (void)
{
   U0TXBUF   =  U0RXBUF;                        // TX -> RXed character
   P5OUT    ^=  0x10;                           // toggle LED
}