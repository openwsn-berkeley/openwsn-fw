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
#define RG_RF09_IRQS                    (0x00)

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
   uint32_t delay;
   uint8_t spi_tx[6];
   uint8_t spi_rx[6];
 
   memset(&app_vars,0,sizeof(app_vars));
   
   // initialize
   board_init();
  
   //set radio control pins to output
  GPIOPinTypeGPIOOutput(GPIO_C_BASE, GPIO_PIN_0);
  GPIOPinTypeGPIOOutput(GPIO_D_BASE, GPIO_PIN_1);
  
  //set radio pwr off
  GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, 0);
  for(delay=0;delay<0xA2C2;delay++);
  
  //init the radio, pwr up the radio
  GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, GPIO_PIN_0);
  for(delay=0;delay<0xA2C2;delay++);
  
  //reset the radio -- inverted so set it to 0.
  GPIOPinWrite(GPIO_D_BASE, GPIO_PIN_1, GPIO_PIN_1);
   
   
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
    
   memset(&spi_tx[0],0,sizeof(spi_tx));
   memset(&spi_rx[0],0,sizeof(spi_rx));
   spi_tx[0] = (AT86RF215_READ_CMD | (uint8_t)(RG_RF09_IRQS >> 8));
   spi_tx[1] = (uint8_t)(RG_RF09_IRQS & 0xFF);             
    
   spi_txrx(
        spi_tx,                     // bufTx
        6,                          // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx,                      // bufRx
        6,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
    
   if (spi_rx[2] == 1){
     leds_sync_on();
   }

  while (1);
   return 0;
}
