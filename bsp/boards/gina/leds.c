/**
\brief GINA-specific definition of the "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init(void) {
   P2DIR      |=  0x0F;                          // P2DIR = 0bxxxx1111 for LEDs
   P2OUT      &= ~0x0F;                          // P2OUT = 0bxxxx0000, all LEDs off
}

// red
void    leds_error_on(void) {
   P2OUT     |=  0x08;
}
void    leds_error_off(void) {
   P2OUT     &= ~0x08;
}
void    leds_error_toggle(void) {
   P2OUT     ^=  0x08;
}
uint8_t leds_error_isOn(void) {
   return (uint8_t)(P2OUT & 0x08)>>3;
}

// orange
void    leds_radio_on(void) {
   P2OUT     |=  0x04;
}
void    leds_radio_off(void) {
   P2OUT     &= ~0x04;
}
void    leds_radio_toggle(void) {
   P2OUT     ^=  0x04;
}
uint8_t leds_radio_isOn(void) {
   return (P2OUT & 0x04)>>4;
}

// green
void    leds_sync_on(void) {
   P2OUT     |=  0x02;
}
void    leds_sync_off(void) {
   P2OUT     &= ~0x02;
}
void    leds_sync_toggle(void) {
   P2OUT     ^=  0x02;
}
uint8_t leds_sync_isOn(void) {
   return (P2OUT & 0x02)>>1;
}

// yellow
void    leds_debug_on(void) {
   P2OUT     |=  0x01;
}
void    leds_debug_off(void) {
   P2OUT     &= ~0x01;
}
void    leds_debug_toggle(void) {
   P2OUT     ^=  0x01;
}
uint8_t leds_debug_isOn(void) {
   return (P2OUT & 0x01)>>0;
}

void leds_all_on(void) {
   P5OUT     &= ~0x0F;
}
void leds_all_off(void) {
   P5OUT     |=  0x0F;
}
void leds_all_toggle(void) {
   P5OUT     ^=  0x0F;
}

void leds_error_blink(void) {
   uint8_t i;
   volatile uint16_t delay;
   // turn all LEDs off
   P2OUT |= 0x08;
     
   // blink error LED for ~10s
   for (i=0;i<80;i++) {
      P2OUT ^=  0x08; //10 seconds more or less..
      for (delay=0xffff;delay>0;delay--);
      for (delay=0xffff;delay>0;delay--);
   }
}

void leds_circular_shift(void) {
   uint8_t temp_leds;
   if ((P2OUT & 0x0F)==0) {                      // if no LEDs on, switch on first one
      P2OUT |= 0x01;
      return;
   }
   temp_leds = P2OUT & 0x0F;                     // retrieve current status of LEDs
   temp_leds <<= 1;                              // shift by one position
   if ((temp_leds & 0x10)!=0) {
      temp_leds++;                               // handle overflow
   }
   P2OUT |= temp_leds;                           // switch on the leds marked '1' in temp_leds
   P2OUT &= ~(~temp_leds & 0x0F);                // switch off the leds marked '0' in temp_leds
}

void leds_increment(void) {
   uint8_t led_counter;
   led_counter = ((P2OUT & 0x0f)+1);
   P2OUT &= ~0x0f; //all LEDs off
   P2OUT |=  led_counter & 0x0f; //LEDs on again
}

//=========================== private =========================================