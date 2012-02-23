/**
\brief Cross-platform declaration "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __SPI_H
#define __SPI_H

#include "stdint.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void    spi_init();
void    spi_txrx(uint8_t* spaceToSend, uint8_t len, uint8_t* spaceToReceive, uint8_t overwrite);

#endif