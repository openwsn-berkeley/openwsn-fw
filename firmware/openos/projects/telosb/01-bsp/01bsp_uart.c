/**
\brief This is a program which shows how to use the bsp modules for the board
       and leds.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "stdio.h"
#include "board.h"
#include "uart.h"
#include "leds.h"

//=========================== defines =========================================

uint8_t stringToSend[] = "Hello World!";

//=========================== variables =======================================

typedef struct {
   uint8_t uart_busyTx;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_uartTxDone();
void cb_uartRxCb();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int main(void)
{  
   board_init();
   
   // setup UART
   uart_txSetup(cb_uartTxDone);
   uart_rxSetup(NULL,
                0,
                0,
                cb_uartRxCb);
   uart_rxStart();
   
   uart_tx(stringToSend,sizeof(stringToSend));
   
   app_vars.uart_busyTx = 1;
   while (app_vars.uart_busyTx==1) {
      board_sleep();
   }
   
   // go back to sleep
   board_sleep();
}

//=========================== callbacks =======================================

void cb_uartTxDone() {
   app_vars.uart_busyTx = 0;
}

void cb_uartRxCb() {
   led_error_toggle();
}