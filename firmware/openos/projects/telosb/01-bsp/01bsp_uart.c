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
   uint8_t uart_rxBuf[10];
   uint8_t uart_busyTx;
   uint8_t uart_rxBytes;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_uartTxDone();
void cb_uartRxCb(uart_event_t ev);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int main(void)
{  
   board_init();
   
   // setup UART
   uart_txSetup(cb_uartTxDone);
   uart_rxSetup(app_vars.uart_rxBuf,
                sizeof(app_vars.uart_rxBuf),
                5,
                cb_uartRxCb);
   uart_rxStart();
   
   uart_tx(stringToSend,sizeof(stringToSend));
   
   app_vars.uart_busyTx = 1;
   while (app_vars.uart_busyTx==1) {
      board_sleep();
   }
   
   while(1) {
      // sleep until bytes received
      app_vars.uart_rxBytes  = 0;
      while (app_vars.uart_rxBytes==0) {
         board_sleep();
      }
      // read bytes from bsp module
      
      // toggle LED for debug
      led_sync_toggle();
   }
}

//=========================== callbacks =======================================

void cb_uartTxDone() {
   app_vars.uart_busyTx      = 0;
}

void cb_uartRxCb(uart_event_t ev) {
   if (ev==UART_EVENT_THRES) {
      led_radio_toggle();
      app_vars.uart_rxBytes  = 1;
   } else {
      led_error_toggle();
   }
}