/**
\brief This program shows the use of the "spi" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

This program was written to communicate with the AT86RF231 radio chip. It will
run regardless of your radio, but might not return anything useful.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "board.h"
#include "spi.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uint8_t    txBuf[3];
   uint8_t    rxBuf[3];
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
   app_vars.txBuf[0]     =  (0x80 | 0x1E);  // [b7]    Read/Write:    1    (read)
                                            // [b6]    RAM/Register : 0    (register)
                                            // [b5-0]  address:       0x1E (Manufacturer ID, Lower 16 Bit)
   app_vars.txBuf[1]     =  0x00;           // send a SNOP strobe just to get the reg value
   app_vars.txBuf[2]     =  0x00;           // send a SNOP strobe just to get the reg value
   
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
