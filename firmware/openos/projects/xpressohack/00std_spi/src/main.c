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

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

int main(void) {
	uint8_t bit;
	
    // power SPI
	LPC_SC->PCONP           |= (1<<10);

    // clock SPI
	LPC_SC->PCLKSEL0        |= (1<<16)|(1<<17);

	// SPI0 Clock counter setting
	LPC_SPI->SPCCR           = 8;

	// pin configuration
	LPC_PINCON->PINSEL0     |= (1<<30)|(1<<31);  // SCK0  = P0.15
	LPC_PINCON->PINSEL1     |= (1<<0 )|(1<<1 );  // SSEL0 = P0.16
	LPC_PINCON->PINSEL1     |= (1<<2 )|(1<<3 );  // MISO0 = P0.17
	LPC_PINCON->PINSEL1     |= (1<<4 )|(1<<5 );  // MOSI0 = P0.18

	LPC_GPIO0->FIODIR       |= (1<<16);
	LPC_GPIO2->FIODIR       |= (1<<6 );


	// SPI in master mode
	LPC_SPI->SPCR           |= (1<<5);           //  master mode

	// enable interrupt
	// NVIC_EnableIRQ(SPI_IRQn);

	bit = 0;
	while(bit<0xff) {
        LPC_GPIO0->FIOCLR   |= (1<<16);
        LPC_GPIO2->FIOCLR   |= (1<<6);

		LPC_SPI->SPDR        = bit;

		while(!(LPC_SPI->SPSR & (1<<7)));

		LPC_GPIO2->FIOSET   |= (1<<6);
		LPC_GPIO0->FIOSET   |= (1<<16);

        bit++;
	}

	// TODO: insert code here

	// Enter an infinite loop, just incrementing a counter
	volatile static int i = 0 ;
	while(1) {
		i++ ;
	}
	return 0 ;
}
