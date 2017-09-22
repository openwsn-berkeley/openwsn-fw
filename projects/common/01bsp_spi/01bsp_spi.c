/**
\brief This program shows the use of the "spi" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

This program was written to communicate with the AT86RF231 radio chip. It will
run regardless of your radio, but might not return anything useful.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
\author Jean-Michel Rubillon <jmrubillon@theiet.org>, June 2017

Modified to communicate with DW1000 radio chip.
TODO: We might want to have a conditional compile option here to allow different
RF chip to be used for checking the SPI functionality is working as expected.

*/

#include "stdint.h"
#include "board.h"
#include "spi.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uint8_t    txBuf[1];
   uint8_t    rxBuf[4];
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {

   memset(&app_vars,0,sizeof(app_vars));
   
   // initialize
   
   board_init();

   // prepare buffer to send over SPI
   app_vars.txBuf[0]     =  0x00;
   
   // retrieve radio manufacturer ID over SPI

   while(1) {
      spi_txrx(
         app_vars.txBuf,
         sizeof(app_vars.txBuf),
         SPI_BUFFER,
         app_vars.rxBuf,
         sizeof(app_vars.rxBuf),
         SPI_FIRST,
         SPI_LAST
      );
   }
}
