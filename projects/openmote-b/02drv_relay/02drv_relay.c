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
#include "sctimer.h"
#include "leds.h"

//=========================== defines =========================================

#define SCTIMER_PERIOD     0xffff // 0xffff@32kHz = 2s
#define MAX_STRING_SIZE     100

uint8_t stringReceived[MAX_STRING_SIZE];
uint8_t writeIndex;

uint8_t payloadString[MAX_STRING_SIZE];
uint8_t payloadSize;


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
uint8_t cb_uartRxCb(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   
   // clear local variable
   memset(&app_vars,0,sizeof(app_vars_t));
   memset(&stringReceived,0,MAX_STRING_SIZE);
   memset(&payloadString,0,MAX_STRING_SIZE);
   writeIndex= 0;
   
   app_vars.uartSendNow = 1;
   
   // initialize the board
   board_init();
   
   // setup UART
   uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
   uart_enableInterrupts();
   
   // setup sctimer
   sctimer_set_callback(cb_compare);
   sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);
   
   
   while(1) {
      
      // wait for timer to elapse
      while (app_vars.uartSendNow==0);
      app_vars.uartSendNow = 0;
      
      // send string over UART
      //app_vars.uartDone              = 0;
      //app_vars.uart_lastTxByteIndex  = 0;
      //uart_writeByte(payloadString[app_vars.uart_lastTxByteIndex]);
      //while(app_vars.uartDone==0);
   }
}

//=========================== callbacks =======================================

void cb_compare(void) {
   
   // have main "task" send over UART
   app_vars.uartSendNow = 1;
   
   // schedule again
   sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);
}

void cb_uartTxDone(void) {
   app_vars.uart_lastTxByteIndex++;
   if (app_vars.uart_lastTxByteIndex<sizeof(payloadString)) {
      uart_writeByte(payloadString[app_vars.uart_lastTxByteIndex]);
   } else {
      app_vars.uartDone = 1;
   }
}

uint8_t cb_uartRxCb(void) {
   uint8_t byte;
   
   
   // toggle LED
   leds_error_toggle();
   
   // read received byte
   
   byte = uart_readByte();
   
   // store the received byte
   if (writeIndex < MAX_STRING_SIZE && byte !='\r')
   {
     stringReceived [writeIndex++] = byte;
   }else{
      // string complete
      payloadSize = writeIndex;
      writeIndex = 0;
      memcpy(payloadString, stringReceived,sizeof(stringReceived)+1);
      memset(&stringReceived,0,sizeof(stringReceived));
   }
   return 0;
}