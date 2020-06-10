#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "spi.h"

void spi_init();

void    spi_txrx(uint8_t*     bufTx,
                 uint16_t     lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint16_t     maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast);