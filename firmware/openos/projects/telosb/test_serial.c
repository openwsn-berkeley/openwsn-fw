 /**
\brief This is a standalone test program for serial communication between the
       TelosB and a computer.

The digital UART interface is:
   - P3.6: UART1TX
   - P3.7: UART1RX

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "msp430f1611.h"
#include "leds.h"

void main(void)
{
  WDTCTL  = WDTPW + WDTHOLD;                     // disable watchdog timer
   
  DCOCTL  = DCO0 | DCO1 | DCO2;                  // MCLK at ~8MHz
  BCSCTL1 = RSEL0 | RSEL1 | RSEL2;               // MCLK at ~8MHz
                                                 // by default, ACLK from 32kHz XTAL which is running
    
  P3SEL     = 0xC0;                              // P3.6,7 = UART1TX/RX
  
  //115200 baud, clocked from 4.8MHz SMCLK
  ME2      |=  UTXE1 + URXE1;                    // enable UART1 TX/RX
  UCTL1    |=  CHAR;                             // 8-bit character
  UTCTL1   |=  SSEL1;                            // clocking from SMCLK
  UBR01     =  41;                               // 4.8MHz/115200 - 41.66
  UBR11     =  0x00;                             //
  UMCTL1    =  0x4A;                             // modulation
  UCTL1    &= ~SWRST;                            // clear UART1 reset bit
  IE2      |=  URXIE1;                           // enable UART1 RX interrupt
  
  __bis_SR_register(LPM0_bits + GIE);            // sleep, leave interrupts on

  /*
  //9600 baud, clocked from 32kHz ACLK
  ME2      |=  UTXE1 + URXE1;                    // enable UART1 TX/RX
  UCTL1    |=  CHAR;                             // 8-bit character
  UTCTL1   |=  SSEL0;                            // clocking from ACLK
  UBR01     =  0x03;                             // 32k/9600 - 3.41
  UBR11     =  0x00;                             //
  UMCTL1    =  0x4A;                             // modulation
  UCTL1    &= ~SWRST;                            // clear UART1 reset bit
  IE2      |=  URXIE1;                           // enable UART1 RX interrupt
  
  __bis_SR_register(LPM3_bits + GIE);            // sleep, leave interrupts and ACLK on
  */
}

#pragma vector=USART1RX_VECTOR
__interrupt void uart_ISR(void)
{
  U1TXBUF = U1RXBUF;                             // TX -> RXed character
  leds_circular_shift();
}