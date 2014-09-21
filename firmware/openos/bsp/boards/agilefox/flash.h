/**
\brief openmoteSTM32 definition of the FLASH.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
#ifndef __FLASH_H
#define __FLASH_H

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================
#include "stdint.h"

void flash_init();
void flash_write_ID();
void flash_read_ID();
void flash_getID(uint8_t* address);
#endif