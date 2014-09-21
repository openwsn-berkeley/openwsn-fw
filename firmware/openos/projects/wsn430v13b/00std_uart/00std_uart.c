 /**
\brief This is a standalone test program for serial communication between the
       TelosB and a computer.

The MSP430 chip speaks 2-wire UART, i.e. one two pins are used: one for sending
bytes (UART1TX), one for receiving bytes (UART1RX). These pins are connected to
an FTDI "UART-to-USB" converted chip. When you plug the TelosB into your
computer, the FTDI driver install a virtual COM port. That is, when you
read/write to that port, the bytes end up on the UART1RX/UART1TX pins.

Connect your TelosB board to your computer, download this application to your
TelosB board and run it. On your computer, open a PuTTY client on the virtual
COM port of your board, and type characters into it.

Each time you type a character, you should see:
- the small green LED (RX) blinks. It's mounted on the RX line, so "shows" you
  when a byte passes.
- the character prints on your terminal, as it is sent back on the TX line.
- the red LED toggles
- the smalll red LED (TX) blinks. It's mounted on the TX line, so "shows" you
  when a byte passes.

Uncomment the BAUDRATE_115200 line below to switch from 9600baud to 115200baud.

The digital UART interface is:
   - P3.6: UART1TX
   - P3.7: UART1RX

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
   P5DIR     |=  0x70;                           // P5DIR = 0bx111xxxx for LEDs
   P5OUT     |=  0x70;                           // P2OUT = 0bx111xxxx, all LEDs off
   
   P3SEL      =  0xC0;                           // P3.6,7 = UART1TX/RX
  
#ifdef BAUDRATE_115200
   //115200 baud, clocked from 4.8MHz SMCLK
   ME2       |=  UTXE1 + URXE1;                  // enable UART1 TX/RX
   UCTL1     |=  CHAR;                           // 8-bit character
   UTCTL1    |=  SSEL1;                          // clocking from SMCLK
   UBR01      =  41;                             // 4.8MHz/115200 - 41.66
   UBR11      =  0x00;                           //
   UMCTL1     =  0x4A;                           // modulation
   UCTL1     &= ~SWRST;                          // clear UART1 reset bit
   IE2       |=  URXIE1;                         // enable UART1 RX interrupt
   
   __bis_SR_register(LPM0_bits + GIE);           // sleep, leave interrupts on
#else
   //9600 baud, clocked from 32kHz ACLK
   ME2       |=  UTXE1 + URXE1;                  // enable UART1 TX/RX
   UCTL1     |=  CHAR;                           // 8-bit character
   UTCTL1    |=  SSEL0;                          // clocking from ACLK
   UBR01      =  0x03;                           // 32768/9600 = 3.41
   UBR11      =  0x00;                           //
   UMCTL1     =  0x4A;                           // modulation
   UCTL1     &= ~SWRST;                          // clear UART1 reset bit
   IE2       |=  URXIE1;                         // enable UART1 RX interrupt
   
   __bis_SR_register(LPM3_bits + GIE);           // sleep, leave interrupts and ACLK on
#endif  
}

/**
\brief This function is called when the the UART module has received a byte.
*/
#pragma vector = USART1RX_VECTOR
__interrupt void USART1RX_ISR (void)
{
   U1TXBUF   =  U1RXBUF;                        // TX -> RXed character
   P5OUT    ^=  0x10;                           // toggle LED
}