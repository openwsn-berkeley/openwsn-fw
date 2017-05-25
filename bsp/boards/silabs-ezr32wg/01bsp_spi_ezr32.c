

#include "stdint.h"
#include "board.h"
#include "spi.h"

#include "ecode.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uint8_t    txBuf[16];
   uint8_t    rxBuf[16];
} app_vars_t;

app_vars_t app_vars;



//=========================== prototypes ======================================

//=========================== main ============================================

/**
\brief The program starts executing here.
 We make use of the spidrv provider by silabs.
 The type enum spi_return_t variable is used to inform to the radio whether we 
 want to send or receive information to/from the radio:
typedef enum {
   SPI_FIRSTBYTE        = 0, receive
   SPI_BUFFER           = 1, send
   SPI_LASTBYTE         = 2, read/write from/to the fifo.
} spi_return_t; 

If we are in the case of read/write; we make use of the type enum spi_first_t 
variable:
typedef enum {
   SPI_NOTFIRST         = 0,  read data
   SPI_FIRST            = 1,  write data
} spi_first_t;
*/
int mote_main(void) {
   
   
  memset(&app_vars,0,sizeof(app_vars));
   
   board_init();
   
   // prepare buffer to send over SPI
   app_vars.txBuf[0]     =  0x01;                                  
   
   // retrieve radio manufacturer ID over SPI
   
      spi_txrx(
         app_vars.txBuf,
         1,
         SPI_BUFFER,
         app_vars.rxBuf,
         1,
         SPI_FIRST,
         SPI_LAST
      );
      
   // get 8 bytes  
         spi_txrx(
         app_vars.txBuf,
         0,
         SPI_FIRSTBYTE,
         app_vars.rxBuf,
         8,
         SPI_FIRST,
         SPI_LAST
          );
         
      return 1;
}
