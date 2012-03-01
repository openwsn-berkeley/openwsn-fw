/**
\brief LPC17XX-specific definition of the "spi" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/

#include "spi.h"
#include "ssp.h"


void    spi_init(){
	SSP0Init();

}

#ifdef SPI_IN_RTOS_MODE
void spi_setCallback(spi_cbt cb) {
   ssp_setCallback(cb);
}
#endif

void    spi_txrx(uint8_t*     bufTx,
                 uint8_t      lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint8_t      maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast){

	SSPTxRcv(0,bufTx,lenbufTx,returnType,bufRx,maxLenBufRx,isFirst,isFirst);



}
