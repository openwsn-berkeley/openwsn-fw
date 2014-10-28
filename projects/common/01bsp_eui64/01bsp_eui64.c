/**
\brief This is a program which shows how to use the bsp modules to read the
   EUI64
       and leds.

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform.

Load this program on your boards. The LEDs should start blinking furiously.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "uart.h"
#include "eui64.h"

//=========================== defines =========================================

uint8_t lookup[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

//=========================== variables =======================================

typedef struct {
              uint8_t eui[8];
              uint8_t eui_string[8*2+7+2];
              uint8_t uart_lastTxByteIndex;
   volatile   uint8_t uartDone;
   volatile   uint8_t uartSendNow;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_uartTxDone(void);
void cb_uartRxCb(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint8_t i;
   uint8_t j;
   
   // initialize the board
   board_init();
   
   // setup UART
   uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
   uart_enableInterrupts();
   
   while(1) {
      
      // clear local variable
      memset(app_vars.eui,0,sizeof(app_vars.eui));
      memset(app_vars.eui_string,'-',sizeof(app_vars.eui_string)-2);
      app_vars.eui_string[23] = '\r';
      app_vars.eui_string[24] = '\n';
      
      // read EUI64
      eui64_get(app_vars.eui);
      
      // format EUI64
      j = 0;
      for (i=0;i<8;i++) {
         app_vars.eui_string[j] = lookup[app_vars.eui[i]>>4 & 0x0f];
         j++;
         app_vars.eui_string[j] = lookup[app_vars.eui[i]>>0 & 0x0f];
         j++;
         j++;
      }
      
      // send string over UART
      app_vars.uartDone              = 0;
      app_vars.uart_lastTxByteIndex  = 0;
      uart_writeByte(app_vars.eui_string[app_vars.uart_lastTxByteIndex]);
      while(app_vars.uartDone==0);
   }
}

//=========================== callbacks =======================================

void cb_uartTxDone(void) {
   app_vars.uart_lastTxByteIndex++;
   if (app_vars.uart_lastTxByteIndex<sizeof(app_vars.eui_string)) {
      uart_writeByte(app_vars.eui_string[app_vars.uart_lastTxByteIndex]);
   } else {
      app_vars.uartDone = 1;
   }
}

void cb_uartRxCb(void) {
}