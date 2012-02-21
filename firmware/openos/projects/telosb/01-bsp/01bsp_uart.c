/**
\brief This is a program which shows how to use the bsp modules for the board
       and leds.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "board.h"
#include "uart.h"

/**
\brief The program starts executing here.
*/
int main(void)
{  
   board_init();
   
   while (1) {
      uart_tx('a');
   }
}