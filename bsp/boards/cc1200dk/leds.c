/**
\brief cc1200dk-specific definition of the "leds" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, August 2016.
*/

#include "msp430f5438a.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void    leds_init() {
    P4DIR |= BIT0 | BIT1 | BIT2 | BIT3 ;                          // P4DIR = 0bxxxx1111 for LEDs
    P4OUT |= BIT0 | BIT1 | BIT2 | BIT3 ;                          // P4OUT = 0bxxxx1111, all LEDs off
   
}

// red = LED1 = P4.0
void    leds_error_on() {
    P4OUT     &= ~BIT0;
}
void    leds_error_off() {
    P4OUT     |=  BIT0;
}
void    leds_error_toggle() {
    P4OUT     ^=  BIT0;
}
uint8_t leds_error_isOn() {
    return (uint8_t)(P4OUT & BIT0);
}
void leds_error_blink() {
    uint8_t i;
    volatile uint16_t delay;
    // turn all LEDs off
    P4OUT     |=  BIT0 | BIT1 | BIT2 | BIT3 ;
   
    // blink error LED for ~10s
    for (i=0;i<80;i++) {
        P4OUT     ^=  BIT0;
        for (delay=0xffff;delay>0;delay--);
    }
}

// yellow = LED2 = P4.1
void    leds_radio_on() {
    P4OUT     &= ~BIT1;
}
void    leds_radio_off() {
    P4OUT     |=  BIT1;
}
void    leds_radio_toggle() {
    P4OUT     ^=  BIT1;
}
uint8_t leds_radio_isOn() {
    return (uint8_t)(P4OUT & BIT1);
}

// green = LED3 = P4.2
void    leds_sync_on() {
    P4OUT     &= ~BIT2;
}
void    leds_sync_off() {
    P4OUT     |=  BIT2;
}
void    leds_sync_toggle() {
    P4OUT     ^=  BIT2;
}
uint8_t leds_sync_isOn() {
    return (uint8_t)(P4OUT & BIT2);
}

// red = LED4 = P4.3
void    leds_debug_on() {
    P4OUT     &= ~BIT3;
}

void    leds_debug_off() {
    P4OUT     |=  BIT3;
}

void    leds_debug_toggle() {
    P4OUT     ^=  BIT3;
}

uint8_t leds_debug_isOn() {
    return (uint8_t)(P4OUT & BIT3);
}

void    leds_all_on() {
    P4OUT     &= ~(BIT0 | BIT1 | BIT2 | BIT3);
}
void    leds_all_off() {
    P4OUT     |=  BIT0 | BIT1 | BIT2 | BIT3;
}
void    leds_all_toggle() {
    P4OUT     ^=  BIT0 | BIT1 | BIT2 | BIT3;
}

void    leds_circular_shift() {
    uint8_t leds_on;
    // get LED state
    leds_on  = (~P4OUT & BIT0 | BIT1 | BIT2 | BIT3) ;
    // modify LED state
    if (leds_on==0) {                             // if no LEDs on, switch on one
        leds_on = BIT0;
    } else {
        leds_on <<= 1;                             // shift by one position
        if ((leds_on & (BIT0 | BIT1 | BIT2 | BIT3))!=0) {
            leds_on &= ~(BIT0 | BIT1 | BIT2 | BIT3);
            leds_on |=  BIT0;                       // handle overflow
        }
    }
    // apply updated LED state
    leds_on <<= 4;                                // send back to position 4
    P4OUT |=  (~leds_on & 0x000f);                  // switch on the leds marked '1' in leds_on
    P4OUT &= ~( leds_on & 0x000f);                  // switch off the leds marked '0' in leds_on
}

void    leds_increment() {
    uint8_t leds_on;
    // get LED state
    leds_on  = (~P4OUT & 0x000f);
    // modify LED state
    if (leds_on==0) {                             // if no LEDs on, switch on one
        leds_on = 0x0001;
    } else {
        leds_on += 1;
    }
    // apply updated LED state
    leds_on <<= 4;                                // send back to position 4
    P4OUT |=  (~leds_on & (0x000f));                  // switch on the leds marked '1' in leds_on
    P4OUT &= ~( leds_on & 0x000f);                  // switch off the leds marked '0' in leds_on
}

//=========================== private =========================================
