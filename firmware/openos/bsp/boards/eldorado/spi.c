/**
\brief GINA-specific definition of the "spi" bsp module.

\author Renato/Fabien, February 2012.
*/

#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "eldorado.h"
#include "spi.h"


//=========================== defines =========================================

//=========================== variables =======================================

uint8_t junk;

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
	SPI1C1 = 0x50;   /*  
	                      *  0b01010000
	                      *    ||||||||__ SPI serial data transfers start with MSB
	                      *    |||||||___ SS1 used as GPIO
	                      *    ||||||____ First edge occurs at the middle of cycle
	                      *    |||||_____ Active-high SPI clock
	                      *    ||||______ SPI Configured as Master
	                      *    |||_______ SPI Transmit Interrupt Disabled
	                      *    ||________ SPI System Enabled
	                      *    |_________ SPI Interrupt Disabled
	                      */
	    SPI1C2 = 0x00;   /*  
	                      *  0b00000000
	                      *    ||||||||__ Separate pins for data input and output.
	                      *    |||||||___ SPI clocks operate in wait mode.
	                      *    ||||||____ Unimplemented
	                      *    |||||_____ SPI data I/O pin acts as an input.
	                      *    ||||______ Mode Fault Disabled
	                      *    |||_______ Unimplemented
	                      *    ||________ Unimplemented
	                      *    |_________ Unimplemented
	                      */
	    SPI1BR = 0x00;   /*  
	                      *  0b000000000
	                      *    |||||||| 
	                      *    ||||||||
	                      *    ||||||++-- Rate Divisor = 2
	                      *    |||||_____ Unimplemented
	                      *    |||| 
	                      *    |||| 
	                      *    ||++------ Prescaler Divisor = 0 
	                      *    |_________ Unimplemented
	                      */
   
   // enable interrupts via the IEx SFRs
#ifdef SPI_IN_INTERRUPT_MODE
	    SPI1C1 = 0xD0;   /*  
	    	                      *  0b11110000
	    	                      *    ||||||||__ SPI serial data transfers start with MSB
	    	                      *    |||||||___ SS1 used as GPIO
	    	                      *    ||||||____ First edge occurs at the middle of cycle
	    	                      *    |||||_____ Active-high SPI clock
	    	                      *    ||||______ SPI Configured as Master
	    	                      *    |||_______ SPI Transmit Interrupt Enabled
	    	                      *    ||________ SPI System Enabled
	    	                      *    |_________ SPI Interrupt Enabled
	    	                      */
#endif
}


#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCallback(spi_cbt cb) {
   spi_vars.callback = cb;
}
#endif

void spi_txrx(uint8_t*     bufTx,
              uint8_t      lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint8_t      maxLenBufRx,
              spi_first_t  isFirst,
              spi_last_t   isLast) {


	   uint8_t u8TempValue;
	   //clear interrupts
	   u8TempValue = SPI1S;
	   u8TempValue = SPI1D;
	   
#ifdef SPI_IN_INTERRUPT_MODE
   // disable interrupts
	MC13192_IRQ_Disable();
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
   

   
   // assert CS signal to have slave listening
   if (spi_vars.isFirst==SPI_FIRST) {
	   MC13192_CE = 0;
   }
   

#ifdef SPI_IN_INTERRUPT_MODE
   // implementation 1. use a callback function when transaction finishes
   
   // write first byte to TX buffer
   SPI1D                 = *spi_vars.pNextTxByte;
   
   // re-enable interrupts
   MC13192_IRQ_Enable();
#else
   // implementation 2. busy wait for each byte to be sent
   
   // send all bytes
   while (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
	   SPI1D   = *spi_vars.pNextTxByte;
      // busy wait on the interrupt flag
	   while (!(SPI1S_SPRF));
      // clear the interrupt flag
      junk = SPI1S;
      // save the byte just received in the RX buffer
      switch (spi_vars.returnType) {
         case SPI_FIRSTBYTE:
            if (spi_vars.numTxedBytes==0) {
               *spi_vars.pNextRxByte   = SPI1D;
            }
            break;
         case SPI_BUFFER:
            *spi_vars.pNextRxByte      = SPI1D;
            spi_vars.pNextRxByte++;
            break;
         case SPI_LASTBYTE:
            *spi_vars.pNextRxByte      = SPI1D;
            break;
      }
      // one byte less to go
      spi_vars.pNextTxByte++;
      spi_vars.numTxedBytes++;
      spi_vars.txBytesLeft--;
   }
   
   // put CS signal high to signal end of transmission to slave
   if (spi_vars.isLast==SPI_LAST) {
	   MC13192_CE = 1;
   }
   
   // SPI is not busy anymore
   spi_vars.busy             =  0;
#endif
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
//will probably throw an error if SPI_IN_INTERRUPT_MODE; Renato can fix like in radio.c
#ifdef SPI_IN_INTERRUPT_MODE
//#pragma vector = VectorNumber_Vspi1
//poipoi syntax
ISR(SPI1_int){
//__interrupt void USCIAB0RX_ISR (void) {
      uint8_t junk;
      // clear the interrupt flag
      junk = SPI1S;
   // save the byte just received in the RX buffer
   switch (spi_vars.returnType) {
      case SPI_FIRSTBYTE:
         if (spi_vars.numTxedBytes==0) {
            *spi_vars.pNextRxByte = SPI1D;
         }
         break;
      case SPI_BUFFER:
         *spi_vars.pNextRxByte    = SPI1D;
         spi_vars.pNextRxByte++;
         break;
      case SPI_LASTBYTE:
         *spi_vars.pNextRxByte    = SPI1D;
         break;
   }
   
   // one byte less to go
   spi_vars.pNextTxByte++;
   spi_vars.numTxedBytes++;
   spi_vars.txBytesLeft--;
   

   
   if (spi_vars.txBytesLeft>0) {
      // write next byte to TX buffer
	   SPI1D              = *spi_vars.pNextTxByte;
   } else {
      // put CS signal high to signal end of transmission to slave
      if (spi_vars.isLast==SPI_LAST) {
    	  MC13192_CE = 1;
      }
      // SPI is not busy anymore
      spi_vars.busy          =  0;
      
      // SPI is done!
      if (spi_vars.callback!=NULL) {
         // call the callback
         spi_vars.callback();
         // make sure CPU restarts after leaving interrupt
         //poipoi __bic_SR_register_on_exit(CPUOFF);
      }
   }
}

#else
ISR(SPI1_int){
	
	
}

#endif
