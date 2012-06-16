/**
\brief eui64 definition of the "eui64" bsp module.

\author Kevin Weekly <kweekly@eecs.berkeley.edu>, June 2012.
*/

#include <avr/io.h>
#include "string.h"
#include "eui64.h"

//=========================== defines =========================================

//=========================== variables =======================================



//=========================== prototypes ======================================

void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {
	int c;

	for ( c =0; c < 8; c++ ) {
		addressToWrite[c] = EEPROM_read(c);	
	}
}

//=========================== private =========================================

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous erase/write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address */
	EEAR = uiAddress;
	EEDR = 255;
	/* Write logical one to EEMPE and enable erase only*/
	EECR = (1<<EEMPE) + (1<<EEPM0);
	/* Start eeprom erase by setting EEPE */
	EECR |= (1<<EEPE);
	/* Wait for completion of erase */
	while(EECR & (1<<EEPE))
	;
	/* Set up Data Registers */
	EEDR = ucData;
	/* Write logical one to EEMPE and enable write only */
	EECR = (1<<EEMPE) + (2<<EEPM0);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}
