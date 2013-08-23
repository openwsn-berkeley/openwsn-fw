/**
\brief This project runs the full OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "board.h"
#include "scheduler.h"
#include "openwsn.h"
#include "uart.h"
#include <stdio.h>

int putchar(int c) {
  do {
    uart_writeByte(c);
    while ((IFG2 & URXIFG1) == 0);
  } while (0);
  return c;
}




int mote_main() {
  uint8_t tmp;
  uint8_t leds_on;
  uart_tx_cbt txCb;
  uart_rx_cbt rxCb;
  
   printf("\n\nOpenwsn test on senslab\n");
  // printf(">Board init...\n");
   board_init();
   // printf(">Board init done.\n");
   //printf(">Scheduler init...\n");
   scheduler_init();
   // printf(">Scheduler init done.\n");
   //  printf(">Openwsn init...\n");
   openwsn_init();
   //  printf(">Openwsn init done.\n");
   //  printf(">Scheduler start.\n");
   scheduler_start();

   uart_setCallbacks(txCb, rxCb);
     
   if (tmp == 'l') {
     printf("Value received_n");
     leds_on = leds_radio_isOn();
     printf("led value : %i\n", leds_on);
       }
   return 0; // this line should never be reached
}
