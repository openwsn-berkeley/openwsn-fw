/**
\brief MoteISTv5-specific definition of the "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
\author Diogo Guerra <diogoguerra@ist.utl.pt>, <dy090.guerra@gmail.com>, July 2015.
*/

#include "hal_MoteISTv5.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================
///<! @Note: MSP430 board ports have negative logic

void    leds_init(void) {
   LED_PORT_SEL &= ~(LED_R | LED_O | LED_G); 
   LED_PORT_DIR |= (LED_R | LED_O | LED_G);
   LED_PORT_OUT |= (LED_R | LED_O | LED_G);
}

// red = LED1 = P4.5
void    leds_error_off(void) {
   LED_PORT_OUT     |=  LED_R;
}
void    leds_error_on(void) {
   LED_PORT_OUT     &= ~LED_R;
}
void    leds_error_toggle(void) {
   LED_PORT_OUT     ^=  LED_R;
}

uint8_t leds_error_isOn(void) {
   return (uint8_t)(~LED_PORT_OUT & LED_R)>>5;
}
void leds_error_blink(void) {
   uint8_t i;
   volatile uint16_t delay;
   // turn all LEDs off (!logic)
   LED_PORT_OUT     |=  (LED_R | LED_O | LED_G);
   
   // blink error LED for ~10s
   for (i=0;i<80;i++) {
      LED_PORT_OUT     ^=  LED_R;
      for (delay=0xffff;delay>0;delay--);
   }
}

// orange = LED2 = P4.6
void    leds_sync_off(void) {
   LED_PORT_OUT     |=  LED_O;
}
void    leds_sync_on(void) {
   LED_PORT_OUT     &= ~LED_O;
}
void    leds_sync_toggle(void) {
   LED_PORT_OUT     ^=  LED_O;
}
uint8_t leds_sync_isOn(void) {
   return (uint8_t)(~LED_PORT_OUT & LED_O)>>6;
}

// green = LED3 = P4.7
void    leds_radio_on(void) {
   LED_PORT_OUT     &= ~LED_G;
}
void    leds_radio_off(void) {
   LED_PORT_OUT     |=  LED_G;
}
void    leds_radio_toggle(void) {
   LED_PORT_OUT     ^=  LED_G;
}

uint8_t leds_radio_isOn(void) {
   return (uint8_t)(~LED_PORT_OUT & LED_G)>>7;
}

// no debug LED on the MoteIST :(
void    leds_debug_on(void) { 
}

void    leds_debug_off(void) {
}

void    leds_debug_toggle(void) {
}

uint8_t leds_debug_isOn(void) {
   return 0;
}

// all leds
void    leds_all_on(void) {
   LED_PORT_OUT     &= ~(LED_R | LED_O | LED_G);
}
void    leds_all_off(void) {
   LED_PORT_OUT     |=  (LED_R | LED_O | LED_G);
}
void    leds_all_toggle(void) {
   LED_PORT_OUT     ^=  (LED_R | LED_O | LED_G);
}

void    leds_circular_shift(void) {
   uint8_t leds_on;
   // get LED state
   leds_on  = (~LED_PORT_OUT & (LED_R | LED_O | LED_G)) >> 5;
   // modify LED state
   if (leds_on==0) {                             // if no LEDs on, switch on one
      leds_on = 0x01;
   } else {
      leds_on <<= 1;                             // shift by one position
      if ((leds_on & 0x08)!=0) {
         leds_on &= ~0x08;
      }
   }
   // apply updated LED state
   leds_on <<= 5;                                // send back to position 5
   LED_PORT_OUT &=  ~((LED_R | LED_O | LED_G));                  // switch on the leds marked '1' in leds_on
   LED_PORT_OUT |=   ( (~leds_on) & (LED_R | LED_O | LED_G));                  // switch off the leds marked '0' in leds_on
}

void    leds_increment(void) {
   uint8_t leds_on;
   // get LED state
   leds_on  = (~LED_PORT_OUT & (LED_R | LED_O | LED_G)) >> 5;
   // modify LED state
   if (leds_on==0) {                             // if no LEDs on, switch on one
      leds_on = 0x01;
   } else {
      leds_on += 1;
   }
   // apply updated LED state
   leds_on <<= 5;                                // send back to position 5

   LED_PORT_OUT &=  ~( (LED_R | LED_O | LED_G));                  // switch on the leds marked '1' in leds_on
   LED_PORT_OUT |=   ( (~leds_on) & (LED_R | LED_O | LED_G));                  // switch off the leds marked '0' in leds_on
}

//=========================== private =========================================
