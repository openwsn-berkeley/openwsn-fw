/**
\brief This is a program which shows how to use the bsp modules for the board
       and leds.

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform.

Load this program on your boards. The LEDs should start blinking furiously.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"

void some_delay(void);

/**
\brief The program starts executing here.
*/
int mote_main(void) {uint8_t i;
   
   board_init();
   
   // error LED functions
   leds_error_on();          some_delay();
   leds_error_off();         some_delay();
   leds_error_toggle();      some_delay();
   leds_error_blink();       some_delay();
   
   // radio LED functions
   leds_radio_on();          some_delay();
   leds_radio_off();         some_delay();
   leds_radio_toggle();      some_delay();
   
   // sync LED functions
   leds_sync_on();           some_delay();
   leds_sync_off();          some_delay();
   leds_sync_toggle();       some_delay();
   
   // debug LED functions
   leds_debug_on();          some_delay();
   leds_debug_off();         some_delay();
   leds_debug_toggle();      some_delay();
   
   // all LED functions
   leds_all_off();           some_delay();
   leds_all_on();            some_delay();
   leds_all_off();           some_delay();
   leds_all_toggle();        some_delay();
   
   // LED increment function
   leds_all_off();           some_delay();
   for (i=0;i<9;i++) {
      leds_increment();      some_delay();
   }
   
   // LED circular shift function
   leds_all_off();           some_delay();
   leds_error_on();          some_delay();
   for (i=0;i<9;i++) {
      leds_circular_shift(); some_delay();
   }
   
   // reset the board, so the program starts running again
   board_reset();
   
   return 0;
}

void some_delay(void) {
   volatile uint16_t delay;
   for (delay=0xffff;delay>0;delay--);
}