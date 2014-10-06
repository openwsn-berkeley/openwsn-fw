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

extern uint8_t addressToWrite[ID_LENGTH];
extern void flash_init(void);

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint8_t flash_reset_val[ID_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
   uint8_t idx;
   uint8_t flash_retry_cnt;
   board_init();
  
   flash_init();
   
   for(flash_retry_cnt = 0; flash_retry_cnt < 3; flash_retry_cnt++)
   {
	   flash_read_ID();
	   if(!(memcmp(flash_reset_val, addressToWrite, ID_LENGTH)))
	   {
		   for (idx = 0; idx < ID_LENGTH; idx++)
		   {
			 addressToWrite[idx] = idx;		   
		   }
		   memcpy(flash_reset_val, addressToWrite, ID_LENGTH);
		   flash_write_ID();
	   }
	   else
	   {
		  break;
	   }
   }
   while (1) {
     
   }
}