/**
green LED: on when UART RX enabled
blue LED:  toggles at each received UART byte
red LED:   successive RX'ed bytes aren't in sequence
*/

#include "msp430f1611.h"
#include "stdint.h"

#define XOFF 0x13
#define XON  0x11

uint8_t fSendNextByte = 0;
uint8_t rxByte;
uint8_t lastRxByte    = 0xff;
uint8_t rxIsEnabled   = 0;
uint8_t fChangeState  = 0;
uint8_t txByte        = 0;

/**
\brief The program starts executing here.
*/
int main(void) {

    // watchdog
    WDTCTL          =  WDTPW + WDTHOLD;          // disable watchdog timer

    // clocking
    DCOCTL          =  DCO0 | DCO1 | DCO2;       // MCLK at 8MHz
    BCSCTL1         =  RSEL0 | RSEL1 | RSEL2;    // MCLK at 8MHz

    // debugpins
    P2DIR |=  0x40;      // task  [P2.6] --> UART RX enabled
    P6DIR |=  0x01;      // isr   [P6.0] --> UART TX interrupt
    P6DIR |=  0x02;      // radio [P6.1] --> UART RX interrupt
    
    // LEDs
    P5DIR          |=  0x70;                     // P5DIR = 0bx111xxxx for LEDs
    P5OUT          |=  0x70;                     // P5OUT = 0bx111xxxx, all LEDs off

    // UART
    P3SEL           =  0xc0;                     // P3.6,7 = UART1TX/RX

    UCTL1           =  SWRST;                    // hold UART1 module in reset
    UCTL1          |=  CHAR;                     // 8-bit character
    
    /*
    UTCTL1         |=  SSEL0;                    // clocking from SMCLK
    UBR01           =  0x03;                     // 32768/9600 = 3.41
    UBR11           =  0x00;                     //
    UMCTL1          =  0x4A;                     // modulation
    */
    
    UTCTL1         |=  SSEL1;                    // clocking from SMCLK
    UBR01           =  41;                       // 4.8MHz/115200 - 41.66
    UBR11           =  0x00;                     //
    UMCTL1          =  0x4A;                     // modulation
    
    ME2            |=  UTXE1 + URXE1;            // enable UART1 TX/RX
    UCTL1          &= ~SWRST;                    // clear UART1 reset bit

    IE2            |=  UTXIE1 | URXIE1;          // enable UART1 TX/RX interrupts

    // TimerA
    TACCTL0         =  CCIE;                     // capture/compare interrupt enable
    TACCR0          =  328;                      // 328@32kHz ~ 10ms
    TACTL           =  MC_1+TASSEL_1;            // up mode, using ACLK

    __bis_SR_register(GIE);                      // enable interrupts

    // send bytes over UART in order
    txByte = 0;
    while (1) {
        
        fSendNextByte   = 0;  
      
        if (fChangeState==0x01) {
            if (rxIsEnabled==0x01) {
                P5OUT       &= ~0x20;            // green LED ON (clear bit)
                P2OUT       |=  0x40;
                U1TXBUF      = XON;              // send XON
            } else {
                P5OUT       |=  0x20;            // green LED OFF (set bit)
                P2OUT       &= ~0x40;
                U1TXBUF      = XOFF;             // send XOFF
            }
            fChangeState=0x00;
        } else {
            txByte++;
            if (txByte==XON || txByte==XOFF) {
                txByte++;
            }
            U1TXBUF      =  txByte;
        }
        
        while (fSendNextByte==0) {
            // wait
        }
    }
}

#pragma vector = USART1TX_VECTOR
__interrupt void USART1TX_ISR (void) {
    P6OUT         ^=  0x01; 
    IFG2          &= ~UTXIFG1;                   // clear UART TX interrupt
    fSendNextByte  = 1;
}

#pragma vector = USART1RX_VECTOR
__interrupt void USART1RX_ISR (void) {
    P6OUT         ^=  0x02;
    IFG2          &= ~URXIFG1;                   // clear UART RX interrupt
    
    P5OUT          ^=  0x40;                     // toggle blue LED
    rxByte          = U1RXBUF;
    if (rxByte!=(uint8_t)(lastRxByte+1)) {
        P5OUT       ^=  0x10;                    // toggle red LED
    }
    lastRxByte      = rxByte;
}

#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR (void) {
    rxIsEnabled    ^= 0x01;
    fChangeState    = 0x01;
}
