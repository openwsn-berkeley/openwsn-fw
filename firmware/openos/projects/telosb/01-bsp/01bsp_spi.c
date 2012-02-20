/**
\brief This program shows the use of the "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "board.h"
#include "spi.h"

/**
\brief The program starts executing here.
*/
int main(void)
{  
   board_init();
   
   board_sleep();
}