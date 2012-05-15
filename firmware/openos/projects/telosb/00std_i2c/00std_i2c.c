 /**
\brief This is a standalone test program for I2C communication between the
       MSP430 micro-controller and the humidity sensor on the TelosB platform.

The digital SPI interface consists of:
   
   - P3.2/SOMI0: "slave-out-master-in", i.e. CC2420->MSP430
   - P3.3/SCL:   clock line
   - P4.2:       chip-select (active-low)
Extra lines:
   - P1.7:     "slave-it-master-out", i.e. MSP430->CC2420

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

// DON'T USE, THIS IS WORK-IN-PROGRESS.

#include "msp430f1611.h"
#include "stdint.h"

void main(void)
{  
   uint16_t   delay;
   
   WDTCTL     =  WDTPW + WDTHOLD;                // disable watchdog timer
   
   DCOCTL     =  DCO0 | DCO1 | DCO2;             // MCLK at ~8MHz
   BCSCTL1    =  RSEL0 | RSEL1 | RSEL2;          // MCLK at ~8MHz
                                                 // by default, ACLK from 32kHz XTAL which is running
   
   P5DIR     |=  0x70;                           // P5DIR = 0bx111xxxx for LEDs
   P5OUT     |=  0x70;                           // P2OUT = 0bx111xxxx, all LEDs off
   
   //===== Step 0. enable and reset the radio
   
   P4DIR     |=  0x20;                           // P4.5 radio VREG enabled, output
   P4OUT     |=  0x20;                           // P4.5 radio VREG enabled, hold high
   
   for (delay=0xffff;delay>0;delay--);           // max. VREG start-up time is 0.6ms
   
   P4DIR     |=  0x40;                           // P4.6 radio reset, output
   P4OUT     &= ~0x40;                           // P4.6 radio reset, hold low
   
   for (delay=0xffff;delay>0;delay--);
   
   P4OUT     |=  0x40;                           // P4.6 radio reset, hold high
   
   for (delay=0xffff;delay>0;delay--);
   
   P5OUT     &= ~0x10;                           // set red LED
   
   //===== Step 1. configure SPI registers
   
   // hold USART state machine in reset mode during configuration
   U0CTL      =  SWRST;                          // [b0] SWRST=1: Enabled. USART logic held in reset state
   
   // configure SPI-related pins
   P3SEL     |=  0x02;                           // P3.1 in SIMO mode
   P3DIR     |=  0x02;                           // P3.1 as output
   P3SEL     |=  0x04;                           // P3.2 in SOMI mode
   P3DIR     |=  0x04;                           // P3.2 as output
   P3SEL     |=  0x08;                           // P3.3 in SCL mode
   P3DIR     |=  0x08;                           // P3.3 as output 
   P4OUT     |=  0x04;                           // P4.2 radio CS, hold high
   P4DIR     |=  0x04;                           // P4.2 radio CS, output
   
   // initialize USART registers
   U0CTL     |=  CHAR | SYNC | MM ;              // [b7]          0: unused
                                                 // [b6]          0: unused
                                                 // [b5]      I2C=0: SPI mode (not I2C)   
                                                 // [b4]     CHAR=1: 8-bit data
                                                 // [b3]   LISTEN=0: Disabled
                                                 // [b2]     SYNC=1: SPI mode (not UART)
                                                 // [b1]       MM=1: USART is master
                                                 // [b0]    SWRST=x: don't change
   
   U0TCTL     =  CKPH | SSEL1 | STC | TXEPT;     // [b7]     CKPH=1: UCLK is delayed by one half cycle
                                                 // [b6]     CKPL=0: normal clock polarity
                                                 // [b5]    SSEL1=1:
                                                 // [b4]    SSEL0=0: SMCLK
                                                 // [b3]          0: unused
                                                 // [b2]          0: unused
                                                 // [b1]      STC=1: 3-pin SPI mode
                                                 // [b0]    TXEPT=1: UxTXBUF and TX shift register are empty                                                 
   
   U0BR1      =  0x00;
   U0BR0      =  0x02;                           // U0BR = [U0BR1<<8|U0BR0] = 2
   U0MCTL     =  0x00;                           // no modulation needed in SPI mode
      
   // enable USART module
   ME1       |=  UTXE0 | URXE0;                  // [b7]    UTXE0=1: USART0 transmit enabled
                                                 // [b6]    URXE0=1: USART0 receive enabled
                                                 // [b5]          x: don't touch!
                                                 // [b4]          x: don't touch!
                                                 // [b3]          x: don't touch!
                                                 // [b2]          x: don't touch!
                                                 // [b1]          x: don't touch!
                                                 // [b0]          x: don't touch!
   
   // clear USART state machine from reset, starting operation
   U0CTL     &= ~SWRST;
   
   // enable interrupts (optional) via the IEx SFRs
   // no need to in this simple implementation (we are busy-waiting)
   
   //===== Step 2. send a SNOP strobe to receive status register
   
   P4OUT     &= ~0x04;                           // P4.2 radio CS, hold low
   
   U0TXBUF    =  0x40;                           // write data over SPI
                                                 // [b7]  RAM/Reg=0: register
                                                 // [b6]      R/W=1: read
                                                 // [b5-0]   reg.=0: SNOP
   
   P5OUT     &= ~0x20;                           // set green LED
   
   while ((IFG1 & URXIFG0)==0);                  // busy wait on the interrupt flag
   
   P5OUT     &= ~0x40;                           // set blue LED
   
   IFG1      &= ~URXIFG0;                        // clear the interrupt flag
   
   P4OUT     |=  0x04;                           // P4.2 radio CS, return to low
   
   __bis_SR_register(LPM4_bits + GIE);           // sleep
}