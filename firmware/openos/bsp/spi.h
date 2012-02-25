/**
\brief Cross-platform declaration "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __SPI_H
#define __SPI_H

#include "stdint.h"

//=========================== typedef =========================================

typedef enum {
   SPI_FIRSTBYTE        = 0,
   SPI_BUFFER           = 1,
   SPI_LASTBYTE         = 2,
} spi_return_t;

typedef enum {
   SPI_NOTFIRST         = 0,
   SPI_FIRST            = 1,
} spi_first_t;

typedef enum {
   SPI_NOTLAST          = 0,
   SPI_LAST             = 1,
} spi_last_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void    spi_init();
void    spi_txrx(uint8_t*     bufTx,
                 uint8_t      lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint8_t      maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast);

#endif