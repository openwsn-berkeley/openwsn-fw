
#include "compiler.h"

void spi_init(void);

void spi_setCallback(spi_cbt cb);

void spi_txrx(uint8_t*  bufTx,
			 uint8_t      lenbufTx,
			 spi_return_t returnType,
			 uint8_t*     bufRx,
			 uint8_t      maxLenBufRx,
			 spi_first_t  isFirst,
             spi_last_t   isLast);
			 
kick_scheduler_t spi_isr(void);