/**
\brief Cross-platform declaration "spi" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __SPI_H
#define __SPI_H

#include "stdint.h"
#include "board.h"

//=========================== define ==========================================

/**
The SPI module functions in two modes:
- in "blocking" mode, all calls return only when the module is done. the CPU
  is not available while the module is busy. This is the preferred method is
  low-RAM system which can not run an RTOS
- in "interrupt" mode, calls are returned after the module has started a
  transaction. When the transaction is done, the module uses a callback
  function to signal this to the caller. This frees up CPU time, allowing for
  other operations to happen concurrently. This is the preferred method when an
  RTOS is present.
*/
//#define SPI_IN_INTERRUPT_MODE

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

typedef void (*spi_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

void    spi_init(void);
#ifdef SPI_IN_INTERRUPT_MODE
void    spi_setCb(spi_cbt cb);
#endif
void    spi_txrx(uint8_t*     bufTx,
                 uint8_t      lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint8_t      maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast);

// interrupt handlers
kick_scheduler_t spi_isr(void);

#endif
