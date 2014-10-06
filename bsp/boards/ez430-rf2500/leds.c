/**
\brief eZ430_RF2500-specific definition of the "leds" bsp module.

\author Chuang Qian <cqian@berkeley.edu>, April 2012.

*/

#include "io430.h"
#include "stdint.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
   P1DIR      |=  0x03;                          // P1DIR = 0bxxxx0011 for LEDs
   P1OUT      &= ~0x03;                          // P1OUT = 0bxxxx0000, all LEDs off
}

// red
void    leds_error_on() {
   P1OUT     |=  0x01;
}
void    leds_error_off() {
   P1OUT     &= ~0x01;
}
void    leds_error_toggle() {
   P1OUT     ^=  0x01;
}
uint8_t leds_error_isOn() {    
   return (P1OUT & 0x01)>>0;
}

// green
void    leds_radio_on() {
   P1OUT     |=  0x02;
}
void    leds_radio_off() {
   P1OUT     &= ~0x02;
}
void    leds_radio_toggle() {
   P1OUT     ^=  0x02;
}
uint8_t leds_radio_isOn() {      
   return (P1OUT & 0x02)>>1;
}


void    leds_sync_on() {
   // eZ430-RF2500 doesn't have a sync LED :(
}
void    leds_sync_off() {
   // eZ430-RF2500 doesn't have a sync LED :(
}
void    leds_sync_toggle() {
   // eZ430-RF2500 doesn't have a sync LED :(
}
uint8_t leds_sync_isOn() {
   // eZ430-RF2500 doesn't have a sync LED :(
   return 0;
}

void    leds_debug_on() {
   // eZ430-RF2500 doesn't have a debug LED :(
}
void    leds_debug_off() {
   // eZ430-RF2500 doesn't have a debug LED :(
}
void    leds_debug_toggle() {
   // eZ430-RF2500 doesn't have a debug LED :(
}
uint8_t leds_debug_isOn() {
   // eZ430-RF2500 doesn't have a debug LED :(
   return 0;
}


void leds_all_on() {
   P1OUT     &= ~0x03;
}
void leds_all_off() {
   P1OUT     |=  0x03;
}
void leds_all_toggle() {
   P1OUT     ^=  0x03;
}

void leds_circular_shift() {
   uint8_t temp_leds;
   if ((P1OUT & 0x03)==0) {                      // if no LEDs on, switch on first one
      P1OUT |= 0x01;
      return;
   }
   temp_leds = P1OUT & 0x03;                     // retrieve current status of LEDs
   temp_leds <<= 1;                              // shift by one position
   if ((temp_leds & 0x03)!=0) {
      temp_leds++;                               // handle overflow
   }
   P1OUT |= temp_leds;                           // switch on the leds marked '1' in temp_leds
   P1OUT &= ~(~temp_leds & 0x03);                // switch off the leds marked '0' in temp_leds
}

void leds_increment() {
   uint8_t led_counter;
   led_counter = ((P1OUT & 0x03)+1);
   P1OUT &= ~0x03; //all LEDs off
   P1OUT |=  led_counter & 0x03; //LEDs on again
}


//=========================== private =========================================