/**
green LED: on when UART RX enabled
blue LED:  toggles at each received UART byte
red LED:   successive RX'ed bytes aren't in sequence
*/

#include "msp430f1611.h"
#include "stdint.h"

#define XOFF 0x13
#define XON  0x11

/**
\brief The program starts executing here.
*/
int main(void) {
   uint8_t txByte;
   // watchdog
   WDTCTL          =  WDTPW + WDTHOLD;           // disable watchdog timer
   
   // clocking
   DCOCTL          =  DCO0 | DCO1 | DCO2;        // MCLK at 8MHz
   BCSCTL1         =  RSEL0 | RSEL1 | RSEL2;     // MCLK at 8MHz
   
   // LEDs
   P5DIR          |=  0x70;                      // P5DIR = 0bx111xxxx for LEDs
   P5OUT          |=  0x70;                      // P5OUT = 0bx111xxxx, all LEDs off
   
   // UART
   P3SEL           =  0xC0;                      // P3.6,7 = UART1TX/RX
   ME2            |=  UTXE1 + URXE1;             // enable UART1 TX/RX
   UCTL1          |=  CHAR;                      // 8-bit character
   UTCTL1         |=  SSEL1;                     // clocking from SMCLK
   UBR01           =  41;                        // 4.8MHz/115200 - 41.66
   UBR11           =  0x00;                      //
   UMCTL1          =  0x4A;                      // modulation
   UCTL1          &= ~SWRST;                     // clear UART1 reset bit
   IE2            |=  URXIE1;                    // enable UART1 RX interrupt
   
   // TimerA
   TACCTL0         =  CCIE;                      // capture/compare interrupt enable
   TACCR0          =  16000;                     // 16000@32kHz ~ 500ms
   TACTL           =  MC_1+TASSEL_1;             // up mode, using ACLK
   
   __bis_SR_register(LPM0_bits + GIE);           // sleep, leave interrupts and ACLK on
   
   /*
   // send bytes over UART in order
   txByte = 0;
   while (1) {
       U1TXBUF     =  ++txByte;
   }
   */
}

/**
\brief This function is called when the the UART module has received a byte.
*/
#pragma vector = USART1RX_VECTOR
__interrupt void USART1RX_ISR (void)
{
   U1TXBUF    =  U1RXBUF;                        // TX -> RXed character
   P5OUT     ^=  0x10;                           // toggle LED
}

/**
\brief This function is called when the TimerA interrupt fires.
*/
#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR (void) {
   P5OUT     ^=  0x20;                           // toggle LED
   if (P5OUT & 0x20) {
      U1TXBUF = XON;                             // enable UART RX
   } else {
      U1TXBUF = XOFF;                            // disable UART RX
   }
}
