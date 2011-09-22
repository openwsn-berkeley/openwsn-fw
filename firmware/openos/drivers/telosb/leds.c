#include "leds.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
   P5DIR  |=  0x70;                              // P5DIR = 0bx111xxxx for LEDs
   P5OUT  &= ~0x70;                              // P2OUT = 0bx000xxxx, all LEDs off
}

//shifts the LEDs by one position
void leds_circular_shift() {
  uint8_t temp_leds;
  if ((P5OUT & 0x70)==0) {                       // if no LEDs on, switch on first one
    P5OUT |= 0x10;
    return;
  }
  temp_leds = P5OUT & 0x70;                      // retrieve current status of LEDs
  temp_leds >>= 4;                               // bring back to position 0, 
  temp_leds <<= 1;                               // shift by one position
  if ((temp_leds & 0x08)!=0) {
    temp_leds |= 1;                              // handle overflow
  }
  temp_leds <<= 4;                               // send back to position 4
  P5OUT |= temp_leds;                            // switch on the leds marked '1' in temp_leds
  P5OUT &= ~(~temp_leds & 0x70);                 // switch off the leds marked '0' in temp_leds
}

//increments the binary number displayed on the LEDs
void leds_increment() {
   uint8_t led_counter;
   led_counter  = (P5OUT & 0x70) >> 4;
   led_counter += 1;
   P5OUT &= ~0x070; //all LEDs off
   P5OUT |=  (led_counter<<4) & 0x70; //LEDs on again
}

//=========================== private =========================================