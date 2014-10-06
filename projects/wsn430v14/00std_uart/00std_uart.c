/**
\brief This is a standalone test program for serial communication between the
   WSN430v14 and a computer.

The MSP430 chip speaks 2-wire UART, i.e. two pins are used: one for sending
bytes (UTXD0), one for receiving bytes (URXD0). When you plug the WSN430
into your computer, it appears as a serial COM port. That is, when you
read/write to that port, the bytes end up on the UTXD0/URXD0 pins.

Connect your WSN430v14 board to your computer, download this application to
your WSN430v14 board and run it. On your computer, open a PuTTY client on the 
COM port of your board, and type characters into it.

Each time you type a character, you should see:
- the character prints on your terminal, as it is sent back on the TX line.
- the red LED toggles.

The digital UART interface is:
- P3.4: UTXD0
- P3.5: URXD0

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2012.
*/

#include "msp430f1611.h"
#include "stdint.h"

/**
\brief The program starts executing here.
*/
void main(void) {
   uint8_t delay;
   
   WDTCTL     =  WDTPW + WDTHOLD;                // disable watchdog timer
   
   //===== clocking
   
   DCOCTL     =  0;                              // we are not using the DCO
   BCSCTL1    =  0;                              // we are not using the DCO
   BCSCTL2    =  SELM_2 | (SELS | DIVS_3) ;      // MCLK=XT2, SMCLK=XT2/8
   
   do {
      IFG1   &= ~OFIFG;                          // clear OSCFault flag
      for (delay=0;delay<0xff;delay++) {         // busy wait for at least 50us
          __no_operation();
      }
   } while ((IFG1 & OFIFG) != 0);                // repeat until OSCFault flag stays cleared
   
   //===== leds
   
   P5DIR     |=  0x70;                           // P5DIR = 0bx111xxxx, all LEDs output
   P5OUT     |=  0x70;                           // P5OUT = 0bx111xxxx, all LEDs off
   
   //===== timer
   
   TACCTL0    =  CCIE;                           // capture/compare interrupt enable
   TACCR0     =  32768;                          // 32768@32kHz = 1000ms
   TACTL      =  MC_1+TASSEL_1;                  // up mode, using ACLK
   
   //===== UART
   
   P3SEL      =  0x30;                           // P3.4,5 = UTXD0/URXD0
   ME1       |=  UTXE0 + URXE0;                  // enable UART0 TX/RX
   U0CTL     |=  CHAR;                           // 8-bit character
   U0TCTL    |=  SSEL1;                          // clocking from SMCLK
   U0BR0      =  0x08;                           // 115200 baud, using 1MHz SMCLK
   U0BR1      =  0x00;
   UMCTL0     =  0x5B;
   U0CTL     &= ~SWRST;                          // clear UART0 reset bit
   
   IE1       |=  URXIE0;                         // enable UART0 RX interrupt
   IE1       |=  UTXIE0;                         // enable UART0 TX interrupt
   
   //===== sleep
   
   __bis_SR_register(LPM0_bits + GIE);           // sleep, leave interrupts on
}

/**
\brief This function is called when the TimerA interrupt fires.
*/
#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void) {
   
   U0TXBUF    =  'a';                            // TX -> RXed character
   P5OUT     ^=  0x20;                           // toggle LED (green)
}

/**
\brief This function is called when the the UART module has received a byte.
*/
#pragma vector = USART0RX_VECTOR
__interrupt void USART0RX_ISR(void) {
   IFG1      &= ~URXIFG0;                        // clear RX interrupt flag
   U0TXBUF    =  U0RXBUF;                        // echo received character
   P5OUT     ^=  0x10;                           // toggle LED (red)
}

/**
\brief This function is called when the the UART module has transmitted a byte.
*/
#pragma vector = USART0TX_VECTOR
__interrupt void USART0TX_ISR(void) {
   IFG1      &= ~UTXIFG0;                        // clear TX interrupt flag
}
