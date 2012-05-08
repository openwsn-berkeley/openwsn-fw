/**
\brief This is a program which shows how to use the bsp modules for the board
       and leds.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "stdio.h"
#include "string.h"
// bsp modules required
#include "board.h"
#include "uart.h"
#include "leds.h"

//=========================== defines =========================================

uint8_t stringToSend[] = "Hello World!";

//=========================== variables =======================================

typedef struct {
   uint8_t uart_lastTxByte;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_uartTxDone();
void cb_uartRxCb();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   board_init();
   
   // clear local variable
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // setup UART
   uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
   uart_enableInterrupts();
   
   // send stringToSend over UART
   app_vars.uart_lastTxByte = 0;
   uart_writeByte(stringToSend[app_vars.uart_lastTxByte]);
   
   while(1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void cb_uartTxDone() {
   app_vars.uart_lastTxByte++;
   if (app_vars.uart_lastTxByte<sizeof(stringToSend)) {
      uart_writeByte(stringToSend[app_vars.uart_lastTxByte]);
   }
}

void cb_uartRxCb() {
   uint8_t byte;
   
   // toggle LED
   leds_debug_toggle();
   
   // read received byte
   byte = uart_readByte();
   
   // echo that byte over serial
   uart_writeByte(byte);
}