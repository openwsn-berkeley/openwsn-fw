/**
\brief Z1-specific definition of the "leds" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2013.
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
   P5DIR      |=  0x70;                          // P5DIR = 0bx111xxxx for LEDs
   P5OUT      |=  0x70;                          // P5OUT = 0bxxxx0000, all LEDs off
}

// red
void    leds_error_on() {
   P5OUT     &=  ~0x10;
}
void    leds_error_off() {
   P5OUT     |=  0x10;
}
void    leds_error_toggle() {
   P5OUT     ^=  0x10;
}
uint8_t leds_error_isOn() {
   return (uint8_t)(P5OUT & 0x10)>>5;
}

// orange
void    leds_radio_on() {
   P5OUT     &= ~0x20;
}
void    leds_radio_off() {
   P5OUT     |= 0x20;
}
void    leds_radio_toggle() {
   P5OUT     ^=  0x20;
}
uint8_t leds_radio_isOn() {
   return (P5OUT & 0x20)>>6;
}

// green
void    leds_sync_on() {
   P5OUT     &= ~0x40;
}
void    leds_sync_off() {
   P5OUT     |= 0x40;
}
void    leds_sync_toggle() {
   P5OUT     ^=  0x40;
}
uint8_t leds_sync_isOn() {
   return (P5OUT & 0x40)>>7;
}

// 3 leds only
void    leds_debug_on() {
  
}
void    leds_debug_off() {
  
}
void    leds_debug_toggle() {
  
}
uint8_t leds_debug_isOn() {
   return 0;
}

void leds_all_on() {
   P5OUT     &= ~0x70;
}
void leds_all_off() {
   P5OUT     |=  0x70;
}
void leds_all_toggle() {
   P5OUT     ^=  0x70;
}

void leds_error_blink() {
   uint8_t i;
   volatile uint16_t delay;
   // turn all LEDs off
   P5OUT |= 0x70;
     
   // blink error LED for ~10s
   for (i=0;i<80;i++) {
      P5OUT ^=  0x10; //10 seconds more or less..
      for (delay=0xffff;delay>0;delay--);
      for (delay=0xffff;delay>0;delay--);
   }
}

void leds_circular_shift() {
   uint8_t temp_leds;
   if ((P5OUT & 0x70)==0) {                      // if no LEDs on, switch on first one
      P5OUT |= 0x10;
      return;
   }
   temp_leds = P5OUT & 0x70;                     // retrieve current status of LEDs
   temp_leds <<= 1;                              // shift by one position
   if ((temp_leds & 0x01)!=0) {
      temp_leds++;                               // handle overflow
   }
   P5OUT |= temp_leds;                           // switch on the leds marked '1' in temp_leds
   P5OUT &= ~(~temp_leds & 0x70);                // switch off the leds marked '0' in temp_leds
}

void leds_increment() {
   uint8_t led_counter;
   led_counter = ((P5OUT & 0x70)+1);
   P5OUT &= ~0x70; //all LEDs off
   P5OUT |=  led_counter & 0x70; //LEDs on again
}

//=========================== private =========================================