/**
\brief SSP-SPI bridge to hide the use of SSP instead of SPI

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/

#ifndef __SSP_SPI_WRAPPER_H
#define __SSP_SPI_WRAPPER_H

#include "stdint.h"


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


//=========================== variables =======================================

//=========================== prototypes ======================================

void    ssp_spi_init();
#ifdef SPI_IN_INTERRUPT_MODE
void    ssp_spi_setCb(spi_cbt cb);
#endif
void    ssp_spi_txrx(uint8_t*     bufTx,
                 uint8_t      lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint8_t      maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast);


#endif
