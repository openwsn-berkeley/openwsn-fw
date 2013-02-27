/**
\brief This is a program which shows how to use the bsp modules for the board
       and leds.

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform.

Load this program on your boards. The LEDs should start blinking furiously.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
#include "flash.h"

/**
\brief The program starts executing here.
*/
int mote_main(void) {
  
   leds_init();
  
   flash_init();
   uint8_t address[8];
   memset(address,0,8);
   flash_write_ID();
   flash_read_ID();
   flash_getID(address);
   while (1) {
   }
}