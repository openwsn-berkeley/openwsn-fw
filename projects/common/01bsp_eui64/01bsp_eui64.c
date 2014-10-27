/**
\brief This is a program which shows how to use the bsp modules to read the
   EUI64
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
#include "eui64.h"

/**
\brief The program starts executing here.
*/
int mote_main(void) {uint8_t i;
   uint8_t eui[8];
   
   board_init();
   
   memset(eui,0,8);
   
   eui64_get(eui);
   
   board_sleep();
   
   return 0;
}
