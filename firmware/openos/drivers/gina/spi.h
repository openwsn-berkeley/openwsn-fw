#ifndef __SPI_H
#define __SPI_H

/**
\addtogroup drivers
\{
\addtogroup SPI
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void    spi_init();
void    spi_write_register(uint8_t reg_addr, uint8_t reg_setting);
uint8_t spi_read_register(uint8_t reg_addr);
void    spi_write_buffer(OpenQueueEntry_t* packet);
void    spi_read_buffer(OpenQueueEntry_t* packet, uint8_t length);

// interrupt handlers
void    isr_spi_rx();

/**
\}
\}
*/

#endif