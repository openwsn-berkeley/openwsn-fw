/**
\brief eZ430_RF2500-specific definition of the "leds" bsp module.


\author Kevin Weekly <kweekly@eecs.berkeley.edu>, June 2012.
*/

#include <avr/io.h>
#include "stdint.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================
#define setb(X,Y) {X |= (1<<Y);}
#define clrb(X,Y) {X &= ~(1<<Y);}
#define togb(X,Y) {X ^= (1<<Y);}

void leds_init(void) {
   DDRE      |=  0x07 << 2;                        
   PORTE     |= (0x07 << 2);    // led's are tied to VCC
   DPDS0	 |= (0x03 << 4);	// port drivers to 8mA
}

// red
void    leds_error_on(void) {
   clrb(PORTE,2);
}
void    leds_error_off(void) {
   setb(PORTE,2);
}
void    leds_error_toggle(void) {
   togb(PORTE,2);
}
uint8_t leds_error_isOn(void) {    
   return ((PORTE & (1<<2)) == 0);
}

// green
void    leds_radio_on(void) {
   clrb(PORTE,3);
}
void    leds_radio_off(void) {
   setb(PORTE,3);
}
void    leds_radio_toggle(void) {
   togb(PORTE,3);
}
uint8_t leds_radio_isOn(void) {      
   return ((PORTE & (1<<3)) == 0);
}


void    leds_sync_on(void) {
   clrb(PORTE,4);
}
void    leds_sync_off(void) {
   setb(PORTE,4);
}
void    leds_sync_toggle(void) {
   togb(PORTE,4);
}
uint8_t leds_sync_isOn(void) {
   return ((PORTE & (1<<4)) == 0);
}

void    leds_debug_on(void) {
   // derfmega doesn't have a debug LED :(
}
void    leds_debug_off(void) {
   // derfmega doesn't have a debug LED :(
}
void    leds_debug_toggle(void) {
   // derfmega doesn't have a debug LED :(
}
uint8_t leds_debug_isOn(void) {
   // eZ430-RF2500 doesn't have a debug LED :(
   return 0;
}


void leds_all_on(void) {
   PORTE     &= ~(0x07<<2);
}
void leds_all_off(void) {
   PORTE     |=  0x07<<2;
}
void leds_all_toggle(void) {
   PORTE     ^=  (0x07<<2);
}

void leds_circular_shift(void) {
   uint8_t temp_leds;
   if ((PORTE & (0x07<<2))==0) {                      // if no LEDs on, switch on first one
      PORTE |= 0x04;
      return;
   }
   temp_leds = (PORTE & (0x07<<2));                     // retrieve current status of LEDs
   temp_leds >>= 1;                              // shift by one position - offset of 2
   if ((temp_leds & 0x07)!=0) {
      temp_leds++;                               // handle overflow
   }
   temp_leds <<= 2;
   PORTE |= temp_leds;                           // switch on the leds marked '1' in temp_leds
   PORTE &= ~(~temp_leds & (0x07<<2));                // switch off the leds marked '0' in temp_leds
}

void leds_increment(void) {
   uint8_t led_counter;
   led_counter = ((PORTE & (0x07<<2))+(1<<2));
   PORTE &= ~(0x07<<2); //all LEDs off
   PORTE |=  led_counter & (0x07<<2); //LEDs on again
}


//=========================== private =========================================