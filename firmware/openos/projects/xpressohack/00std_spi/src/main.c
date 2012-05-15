/**
\brief This is a standalone test program for the SPI on the xpressohack
       board.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

int main(void) {
   uint8_t byte;
   
   // power SPI
   LPC_SC->PCONP            |= (1<<10);

   // clock SPI
   LPC_SC->PCLKSEL0         |= (1<<16)|(1<<17);

   // SPI0 Clock counter setting
   LPC_SPI->SPCCR            = 8;

   // pin configuration
   LPC_PINCON->PINSEL0      |= (1<<30)|(1<<31);  // [P0.15] SCK0
   LPC_PINCON->PINSEL1      |= (1<<2 )|(1<<3 );  // [P0.17] MISO0
   LPC_PINCON->PINSEL1      |= (1<<4 )|(1<<5 );  // [P0.18] MOSI0
   LPC_GPIO2->FIODIR        |= (1<<6 );          // [P2.6]  SSEL0

   // SPI in master mode
   LPC_SPI->SPCR            |= (1<<5);

   // enable interrupt
   // NVIC_EnableIRQ(SPI_IRQn);

   byte = 0;
   while(byte<0xff) {
      // CS low
      LPC_GPIO2->FIOCLR     |= (1<<6);

      // send byte
      LPC_SPI->SPDR          = byte;

      // wait for byte being sent
      while(!(LPC_SPI->SPSR & (1<<7)));

      // CS high
      LPC_GPIO2->FIOSET     |= (1<<6);

      byte++;
   }
}
