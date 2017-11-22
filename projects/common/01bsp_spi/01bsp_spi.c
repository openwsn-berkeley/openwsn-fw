/**
\brief This program shows the use of the "spi" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

This program was written to communicate with the AT86RF215 radio chip. It will
run regardless of your radio, but might not return anything useful.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
\author Xavi Vlajosana <xvilajosana@eecs.berkeley.edu>, Dec 2017.
*/

#include "stdint.h"
#include "board.h"
#include "spi.h"
#include "leds.h"
#include "gpio.h"
#include <headers/hw_memmap.h>


#define AT86RF215_READ_CMD              ( 0x00 )
#define AT86RF215_WRITE_CMD             ( 0x80 )

#define AT86RF215_PN_ADDR               ( 0x000D )
#define AT86RF215_PN_215                ( 0x34 )

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
   uint8_t parnumber;
   uint8_t address_hi, address_lo;
   
   memset(&app_vars,0,sizeof(app_vars));
   
   // initialize
   board_init();
  
   address_hi = (uint8_t) ((AT86RF215_PN_ADDR & 0xFF00) >> 8);
   address_lo = (uint8_t) ((AT86RF215_PN_ADDR & 0x00FF) >> 0);
    
   // prepare buffer to send over SPI
   app_vars.txBuf[0] = AT86RF215_READ_CMD | address_hi;
   app_vars.txBuf[1] = address_lo;
                                      
   app_vars.txBuf[2]     =  0x00;           // send a SNOP strobe just to get the reg value
  
   // retrieve radio manufacturer ID over SPI
   spi_txrx(
         app_vars.txBuf,
         sizeof(app_vars.txBuf),
         SPI_BUFFER,
         app_vars.rxBuf,
         sizeof(app_vars.rxBuf),
         SPI_FIRST,
         SPI_LAST
      );

   parnumber = app_vars.rxBuf[2];

   if (parnumber == AT86RF215_PN_215){
     leds_radio_on();
   }
   while(1);      
}
