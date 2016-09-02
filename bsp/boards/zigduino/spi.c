/**
\brief Zigduino definition of the "spi" bsp module.

\author Sven Akkermans <sven.akkermans@cs.kuleuven.be>, September 2015.
*/

#include "spi.h"
#include "leds.h"

//=========================== defines =========================================
#define HAL_SPI_TRANSFER_WRITE(to_write) (SPDR = (to_write))
#define HAL_SPI_TRANSFER_WAIT() ({while ((SPSR & (1 << SPIF)) == 0) {;}}) /* gcc extension, alternative inline function */
#define HAL_SPI_TRANSFER_READ() (SPDR)
#define HAL_SPI_TRANSFER(to_write) (	  \
				    HAL_SPI_TRANSFER_WRITE(to_write),	\
				    HAL_SPI_TRANSFER_WAIT(),		\
				    HAL_SPI_TRANSFER_READ() )

//=========================== variables =======================================

typedef struct {
   // information about the current transaction
   uint8_t*        pNextTxByte;
   uint8_t         numTxedBytes;
   uint8_t         txBytesLeft;
   spi_return_t    returnType;
   uint8_t*        pNextRxByte;
   uint8_t         maxRxBytes;
   spi_first_t     isFirst;
   spi_last_t      isLast;
   // state of the module
   uint8_t         busy;
#ifdef SPI_IN_INTERRUPT_MODE
   // callback when module done
   spi_cbt         callback;
#endif
} spi_vars_t;

spi_vars_t spi_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void spi_init() {
   // clear variables
   memset(&spi_vars,0,sizeof(spi_vars_t));

   // hold USART state machine in reset mode during configuration

//   // configure SPI-related pins
//	DDRB |= (1<<PB0)|(1<<PB2);  // SCK, MOSI and SS as outputs
//	DDRB &= ~(1<<PB3);          // MISO as input
//
//	SPCR |= (1<<MSTR);         // Set as Master
//	SPCR |= (1<<SPR0);         // divided clock by 16
//
//	SPCR |= (1<<CPHA)|(1<<CPOL);     //Configures correct clock polarity and data is sampled on rising edge
//	SPCR |= (1<<SPE);                // Enable SPI
//
//	PORTB |= (1<<PB0);
//	PORTB &= ~(1<<PB0);
//	PORTB |= (1<<PB0);
//	PORTB &= ~(1<<PB0);
//	PORTB |= (1<<PB0);
//	PORTB &= ~(1<<PB0);        //Toggles SS high-low 3 times to enable SPI

   // initialize USART registers

   // enable USART module

   // clear USART state machine from reset, starting operation

   // enable interrupts via the IEx SFRs

#ifdef SPI_IN_INTERRUPT_MODE
#endif
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCb(spi_cbt cb) {
   spi_vars.spi_cb = cb;
}
#endif

void spi_txrx(uint8_t*     bufTx,
              uint8_t      lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint8_t      maxLenBufRx,
              spi_first_t  isFirst,
              spi_last_t   isLast) {
	leds_error_toggle();
#ifdef SPI_IN_INTERRUPT_MODE
   // disable interrupts
   __disable_interrupt();
#endif

   // register spi frame to send
   spi_vars.pNextTxByte      =  bufTx;
   spi_vars.numTxedBytes     =  0;
   spi_vars.txBytesLeft      =  lenbufTx;
   spi_vars.returnType       =  returnType;
   spi_vars.pNextRxByte      =  bufRx;
   spi_vars.maxRxBytes       =  maxLenBufRx;
   spi_vars.isFirst          =  isFirst;
   spi_vars.isLast           =  isLast;

   // SPI is now busy
   spi_vars.busy             =  1;

   // lower CS signal to have slave listening
   // lower CS signal to have slave listening
   if (spi_vars.isFirst==SPI_FIRST) {
		PORTB &= ~(1<<PB0);      //Set SS low
   }
#ifdef SPI_IN_INTERRUPT_MODE
   // implementation 1. use a callback function when transaction finishes

   // write first byte to TX buffer

   // re-enable interrupts
   __enable_interrupt();
#else
   // implementation 2. busy wait for each byte to be sent

   // send all bytes
   while (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
	   SPDR = *spi_vars.pNextTxByte;
	   // busy wait on the interrupt flag
	   while(!(SPSR & (1<<SPIF)))
	   // clear the interrupt flag
		   	  //UNNEEDED I THINK
      // save the byte just received in the RX buffer
		   while(!(SPSR & (1<<SPIF)))
		   ;
		   /* Return Data Register */
		   return SPDR;
      // one byte less to go
      spi_vars.pNextTxByte++;
      spi_vars.numTxedBytes++;
      spi_vars.txBytesLeft--;
   }

   // put CS signal high to signal end of transmission to slave
	PORTB |= (1<<PB0);	       //Set SS high, signals end of transmission
	PORTB |= (1<<PB5);
	PORTB &= ~(1<<PB5);

   // SPI is not busy anymore
   spi_vars.busy             =  0;
#endif
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t spi_isr() {
#ifdef SPI_IN_INTERRUPT_MODE
   // save the byte just received in the RX buffer

   // one byte less to go
   spi_vars.pNextTxByte++;
   spi_vars.numTxedBytes++;
   spi_vars.txBytesLeft--;

   if (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
   } else {
      // put CS signal high to signal end of transmission to slave

	   // SPI is not busy anymore
      spi_vars.busy             =  0;

      // SPI is done!
      if (spi_vars.callback!=NULL) {
         // call the callback
         spi_vars.spi_cb();
         // kick the OS
         return KICK_SCHEDULER;
      }
   }
   return DO_NOT_KICK_SCHEDULER;
#else
   // this should never happpen!

   // we can not print from within the BSP. Instead:
   // blink the error LED
   leds_error_blink();
   // reset the board
   board_reset();

   return DO_NOT_KICK_SCHEDULER; // we will not get here, statement to please compiler
#endif
}
