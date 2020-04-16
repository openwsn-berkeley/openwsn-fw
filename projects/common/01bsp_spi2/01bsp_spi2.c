/**
\brief This program shows the use of the "spi" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

This program was written to communicate with the AT86RF231 radio chip. It will
run regardless of your radio, but might not return anything useful.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "board.h"
#include "spi.h"
#include "uart.h"
#include "sctimer.h"
#include "leds.h"

//=========================== defines =========================================

#define SCTIMER_PERIOD     0xffff // 0xffff@32kHz = 2s
uint8_t stringToSend[]       = "Hello, World!\r\n";

//=========================== variables =======================================

typedef struct {
   uint8_t    txBuf[3];
   uint8_t    rxBuf[3];
} app_vars_spi_t;

app_vars_spi_t app_vars_spi;

typedef struct {
              uint8_t uart_lastTxByteIndex;
   volatile   uint8_t uartDone;
   volatile   uint8_t uartSendNow;
} app_vars_uart_t;

app_vars_uart_t app_vars_uart;


//=========================== prototypes ======================================

void cb_compare(void);
void cb_uartTxDone(void);
void cb_uartRxCb(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   
   memset(&app_vars_spi,0,sizeof(app_vars_spi_t));
   memset(&app_vars_uart,0,sizeof(app_vars_uart_t));

   app_vars_uart.uartSendNow = 1;
   
   // initialize
   
   board_init();

   // setup UART
   uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
   uart_enableInterrupts();
   
   // setup sctimer
   sctimer_set_callback(cb_compare);
   sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);

   // prepare buffer to send over SPI
   app_vars_spi.txBuf[0]     =  (0x80 | 0x1E);  // [b7]    Read/Write:    1    (read)
                                            // [b6]    RAM/Register : 0    (register)
                                            // [b5-0]  address:       0x1E (Manufacturer ID, Lower 16 Bit)
   app_vars_spi.txBuf[1]     =  0x00;           // send a SNOP strobe just to get the reg value
   app_vars_spi.txBuf[2]     =  0x00;           // send a SNOP strobe just to get the reg value
   
   // retrieve radio manufacturer ID over SPI
   while(1) {
      spi_txrx(
         app_vars_spi.txBuf,
         sizeof(app_vars_spi.txBuf),
         SPI_BUFFER,
         app_vars_spi.rxBuf,
         sizeof(app_vars_spi.rxBuf),
         SPI_FIRST,
         SPI_LAST
      );

      // wait for timer to elapse
      while (app_vars_uart.uartSendNow==0);
      app_vars_uart.uartSendNow = 0;
      
      // send string over UART
      app_vars_uart.uartDone              = 0;
      app_vars_uart.uart_lastTxByteIndex  = 0;
      uart_writeByte(app_vars_spi.rxBuf[app_vars_uart.uart_lastTxByteIndex]);
      while(app_vars_uart.uartDone==0);
   }
}


//=========================== callbacks =======================================

void cb_compare(void) {
   
   // have main "task" send over UART
   app_vars_uart.uartSendNow = 1;
   
   // schedule again
   sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);
}

void cb_uartTxDone(void) {
   app_vars_uart.uart_lastTxByteIndex++;
   if (app_vars_uart.uart_lastTxByteIndex<sizeof(stringToSend)) {
      uart_writeByte(app_vars_spi.rxBuf[app_vars_uart.uart_lastTxByteIndex]);
   } else {
      app_vars_uart.uartDone = 1;
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