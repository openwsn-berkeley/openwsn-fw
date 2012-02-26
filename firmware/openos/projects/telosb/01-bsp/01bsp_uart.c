/**
\brief This is a program which shows how to use the bsp modules for the board
       and leds.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "board.h"
#include "uart.h"

//=========================== defines =========================================

uint8_t stringToSend[] = "Hello World!";

//=========================== variables =======================================

typedef struct {
   uint8_t uart_busy;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_uartDone();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int main(void)
{  
   board_init();
   
   uart_setTxDoneCb(cb_uartDone);
   
   uart_tx(&stringToSend[0],sizeof(stringToSend));
   
   app_vars.uart_busy = 1;
   while (app_vars.uart_busy==1) {
   }
   
   // go back to sleep
   board_sleep();
}

//=========================== callbacks =======================================

void cb_uartDone() {
   app_vars.uart_busy = 0;
}