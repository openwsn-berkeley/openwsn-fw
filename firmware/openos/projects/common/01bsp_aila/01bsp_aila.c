/**
\brief This is a program which shows how to use the bsp modules for the board
       and UART.

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform.

Load this program on your board. Open a serial terminal client (e.g. PuTTY or
TeraTerm):
- You will read "Hello World!" printed over and over on your terminal client.
- when you enter a character on the client, the board echoes it back (i.e. you
  see the character on the terminal client) and the "ERROR" led blinks.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "stdio.h"
#include "string.h"
// bsp modules required
#include "board.h"
#include "uart.h"
#include "bsp_timer.h"
#include "leds.h"

//=========================== defines =========================================

#define BSP_TIMER_PERIOD     0xffff // 0xffff@32kHz = 2s
uint8_t stringToSend[]       = "Hello, World!\r\n";

//=========================== variables =======================================

typedef struct {
              uint8_t uart_lastTxByteIndex;
   volatile   uint8_t uartDone;
   volatile   uint8_t uartSendNow;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_compare(void);
void cb_uartTxDone(void);
void cb_uartRxCb(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   
   // clear local variable
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize the board
   board_init();
   
   // setup UART
   uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
   uart_enableInterrupts();
   
   // setup BSP timer
   bsp_timer_set_callback(cb_compare);
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
   
   while(1) {
      
      // wait for timer to elapse
      while (app_vars.uartSendNow==0);
      app_vars.uartSendNow = 0;
      
      // send string over UART
      app_vars.uartDone              = 0;
      app_vars.uart_lastTxByteIndex  = 0;
      uart_writeByte(stringToSend[app_vars.uart_lastTxByteIndex]);
      while(app_vars.uartDone==0);
   }
}

//=========================== callbacks =======================================

void cb_compare(void) {
   
   // have main "task" send over UART
   app_vars.uartSendNow = 1;
   
   // schedule again
   bsp_timer_scheduleIn(BSP_TIMER_PERIOD);
}

void cb_uartTxDone(void) {
   app_vars.uart_lastTxByteIndex++;
   if (app_vars.uart_lastTxByteIndex<sizeof(stringToSend)) {
      uart_writeByte(stringToSend[app_vars.uart_lastTxByteIndex]);
   } else {
      app_vars.uartDone = 1;
   }
}

void cb_uartRxCb(void) {
   uint8_t byte;
   
   // toggle LED
   leds_error_toggle();
   
   // read received byte
   byte = uart_readByte();
   
   // echo that byte over serial
   uart_writeByte(byte);
}
