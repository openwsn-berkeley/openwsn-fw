#include "leds.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
   P2DIR  |=  0x0F;                              // P2DIR = 0bxxxx1111 for LEDs
   P2OUT  &= ~0x0F;                              // P2OUT = 0bxxxx0000, all LEDs off
}

//shifts the LEDs by one position
void leds_circular_shift() {
  uint8_t temp_leds;
  if ((P2OUT & 0x0F)==0) {                       // if no LEDs on, switch on first one
    P2OUT |= 0x01;
    return;
  }
  temp_leds = P2OUT & 0x0F;                      // retrieve current status of LEDs
  temp_leds <<= 1;                               // shift by one position
  if ((temp_leds & 0x10)!=0) {
    temp_leds++;                                 // handle overflow
  }
  P2OUT |= temp_leds;                            // switch on the leds marked '1' in temp_leds
  P2OUT &= ~(~temp_leds & 0x0F);                 // switch off the leds marked '0' in temp_leds
}

//increments the binary number displayed on the LEDs
void leds_increment() {
   uint8_t led_counter;
   led_counter = ((P2OUT & 0x0f)+1);
   P2OUT &= ~0x0f; //all LEDs off
   P2OUT |=  led_counter & 0x0f; //LEDs on again
}

//=========================== private =========================================