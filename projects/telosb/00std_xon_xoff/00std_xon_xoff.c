/**
green LED: on when UART RX enabled
blue LED:  toggles at each received UART byte
red LED:   successive RX'ed bytes aren't in sequence
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "string.h"

//=========================== defines =========================================

#define XOFF                  0x13
#define XON                   0x11
#define BURSTSIZE              200
#define SLOTSIZE               328 // 328@32kHz ~ 10010us
#define SERIALINHIBITGUARD      16 //  16@32kHz ~   488us
#define DURATIONISR            300 // 300==892us (measured)

//=========================== variables =======================================

typedef struct {
    uint8_t busyBurst;
    uint8_t fSendNextByte;
    uint8_t rxByte;
    uint8_t lastRxByte;
    uint8_t ctsStateChanged;
    uint8_t fInhibited;
    uint8_t txByte;
} appvars_t;

appvars_t appvars;

//=========================== main ============================================

int main(void) {
    
    //=== watchdog
    WDTCTL                        =  WDTPW + WDTHOLD;           // disable watchdog timer
    
    //=== initialize variables
    memset(&appvars,0,sizeof(appvars_t));
    appvars.lastRxByte            = 0xff;
    
    //=== clocking
    DCOCTL                        =  DCO0 | DCO1 | DCO2;        // MCLK at 8MHz
    BCSCTL1                       =  RSEL0 | RSEL1 | RSEL2;     // MCLK at 8MHz
    
    //=== debugpins
    P6DIR                        |=  0x80;                      // slot       [P6.7] --> high==XON, low=XOFF
    P2DIR                        |=  0x40;                      // task       [P2.6] --> task
    P6DIR                        |=  0x01;                      // isr        [P6.0] --> TimerA ISR
    P3DIR                        |=  0x20;                      // isruarttx  [P3.5] --> UART TX ISR
    P3DIR                        |=  0x10;                      // isruartrx  [P3.4] --> UART RX ISR
    
    //=== LEDs
    P5DIR                        |=  0x70;                      // P5DIR = 0bx111xxxx for LEDs
    P5OUT                        |=  0x70;                      // P5OUT = 0bx111xxxx, all LEDs off
    
    //=== UART
    P3SEL                         =  0xc0;                      // P3.6,7 = UART1TX/RX
    
    UCTL1                         =  SWRST;                     // hold UART1 module in reset
    UCTL1                        |=  CHAR;                      // 8-bit character
    
    /*
    UTCTL1                       |=  SSEL0;                     // clocking from SMCLK
    UBR01                         =  0x03;                      // 32768/9600 = 3.41
    UBR11                         =  0x00;                      //
    UMCTL1                        =  0x4A;                      // modulation
    */
    
    UTCTL1                       |=  SSEL1;                     // clocking from SMCLK
    UBR01                         =  41;                        // 4.8MHz/115200 - 41.66
    UBR11                         =  0x00;                      //
    UMCTL1                        =  0x4A;                      // modulation
    
    ME2                          |=  UTXE1 + URXE1;             // enable UART1 TX/RX
    UCTL1                        &= ~SWRST;                     // clear UART1 reset bit
    
    IE2                          |=  UTXIE1 | URXIE1;           // enable UART1 TX/RX interrupts
    
    //=== TimerB
    TBCCR0                        =  SLOTSIZE-1;                // CCR0 contains period of counter
    TBCCTL2                       =  CCIE;                      // CCR2 in compare mode
    TBCCR2                        =  SLOTSIZE-SERIALINHIBITGUARD;
    TBCTL                         =  TBIE+TBCLR;                // interrupt when counter resets
    TBCTL                        |=  MC_1+TBSSEL_1;             // up mode, clocked from ACLK
    
    __bis_SR_register(GIE+LPM0_bits);                           // sleep, but leave clocks on
}

//=========================== helpers =========================================

void sendXONXOFF(void) {
    if (appvars.fInhibited==0x01) {
        P5OUT                    |=  0x20;                      // green LED OFF (set bit)
        P6OUT                    &= ~0x80;                      // clear slot debupin
        U1TXBUF                   =  XOFF;                      // send XOFF
    } else {
        P5OUT                    &= ~0x20;                      // green LED ON (clear bit)
        U1TXBUF                   =  XON;                       // send XON
        P6OUT                    |=  0x80;                      // set slot debupin
    }
}

//=========================== interrupt handlers ==============================

#pragma vector = USART1TX_VECTOR
__interrupt void USART1TX_ISR (void) {
    P3OUT                        |=  0x20;                      // set isruarttx debugpin
    IFG2                         &= ~UTXIFG1;                   // clear UART TX interrupt
    
    if (appvars.ctsStateChanged==1) {
        sendXONXOFF();
        appvars.ctsStateChanged   = 0;
    } else {
        if (appvars.fInhibited==0x01) {
            appvars.busyBurst     = 0;
        } else {
            if (appvars.txByte>BURSTSIZE) {
                // done sending the burst
                appvars.busyBurst = 0;
                appvars.txByte    = 0;
            } else {
                // send next byte in burst
                appvars.busyBurst = 1;
                U1TXBUF           =  appvars.txByte;
                appvars.txByte++;
                if (appvars.txByte==XON || appvars.txByte==XOFF) {
                    appvars.txByte++;
                }
            }
        }
    }
    
    P3OUT                        &= ~0x20;                      // clear isruarttx debugpin
}

#pragma vector = USART1RX_VECTOR
__interrupt void USART1RX_ISR (void) {
    P3OUT                        |=  0x10;                      // set isruartrx debugpin
    IFG2                         &= ~URXIFG1;                   // clear UART RX interrupt
    
    P5OUT                        ^=  0x40;                      // toggle blue LED
    
    // verify rxByte is the one expected
    appvars.rxByte                =  U1RXBUF;
    if (appvars.rxByte!=(uint8_t)(appvars.lastRxByte+1)) {
        P5OUT                    ^=  0x10;                      // toggle red LED
    }
    appvars.lastRxByte            =  appvars.rxByte;
    
    P3OUT                        &= ~0x10;                      // clear isruartrx debugpin
}

#pragma vector = TIMERB1_VECTOR
__interrupt void TIMERB1_ISR (void) {
    volatile uint16_t i;
    uint16_t tbiv_local;
    
    P6OUT                        |=  0x01;                      // set isr debugpin
    
    // reading TBIV returns the value of the highest pending interrupt flag
    // and automatically resets that flag. We therefore copy its value to the
    // tbiv_local local variable exactly once. If there is more than one 
    // interrupt pending, we will reenter this function after having just left
    // it.
    tbiv_local = TBIV;
    
    switch (tbiv_local) {
        case 0x0002: // CCR1 fires
            break;
        case 0x0004: // CCR2 fires (compare)
            // control XON/XOFF
            appvars.fInhibited           ^=  0x01;
            if (appvars.busyBurst==1) {
                appvars.ctsStateChanged   = 1;
            } else {
                sendXONXOFF();
            }
            break;
        case 0x0006: // CCR3 fires
            break;
        case 0x0008: // CCR4 fires
            break;
        case 0x000a: // CCR5 fires
            break;
        case 0x000c: // CCR6 fires
            break;
        case 0x000e: // CCR0 fires (timer overflow)
            // make the ISR artificially long
            if (appvars.fInhibited==0x01) {
                for (i=0;i<DURATIONISR;i++);
            }
            break;
    }
    
    P6OUT                        &= ~0x01;                      // clear isr debugpin
}
