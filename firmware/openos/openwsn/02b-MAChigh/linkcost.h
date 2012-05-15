#ifndef __LINKCOST_H
#define __LINKCOST_H


#include "openwsn.h"

/**
\addtogroup MAChigh
\{
\addtogroup LinkCost
\{
*/

uint8_t linkcost_calcETX(uint8_t numTX, uint8_t numTXACK);//calculate link cost based on ETX.
uint8_t linkcost_calcRSSI();
//other metrics can be used.


#endif
