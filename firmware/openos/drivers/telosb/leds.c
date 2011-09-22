#include "leds.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
   P5DIR  |=  0x70;                              // P5DIR = 0bx111xxxx for LEDs
   P5OUT  |=  0x70;                              // P2OUT = 0bx111xxxx, all LEDs off
}

//shifts the LEDs by one position
void leds_circular_shift() {
   uint8_t leds_on;
   // get LED state
   leds_on  = (~P5OUT & 0x70) >> 4;
   // modify LED state
   if (leds_on==0) {                             // if no LEDs on, switch on one
      leds_on = 0x01;
   } else {
      leds_on <<= 1;                             // shift by one position
      if ((leds_on & 0x08)!=0) {
         leds_on &= ~0x08;
         leds_on |=  0x01;                       // handle overflow
      }
   }
   // apply updated LED state
   leds_on <<= 4;                                // send back to position 4
   P5OUT |=  (~leds_on & 0x70);                  // switch on the leds marked '1' in leds_on
   P5OUT &= ~( leds_on & 0x70);                  // switch off the leds marked '0' in leds_on
}

//increments the binary number displayed on the LEDs
void leds_increment() {
   uint8_t leds_on;
   // get LED state
   leds_on  = (~P5OUT & 0x70) >> 4;
   // modify LED state
   if (leds_on==0) {                             // if no LEDs on, switch on one
      leds_on = 0x01;
   } else {
      leds_on += 1;
   }
   // apply updated LED state
   leds_on <<= 4;                                // send back to position 4
   P5OUT |=  (~leds_on & 0x70);                  // switch on the leds marked '1' in leds_on
   P5OUT &= ~( leds_on & 0x70);                  // switch off the leds marked '0' in leds_on
}

//=========================== private =========================================