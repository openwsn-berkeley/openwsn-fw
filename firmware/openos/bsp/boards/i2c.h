#ifndef __I2C_H
#define __I2C_H

/**
\addtogroup drivers
\{
\addtogroup I2C
\{
*/

#include "stdint.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void i2c_init(void);

void i2c_read_byte(uint8_t address, uint8_t* byte);
void i2c_read_bytes(uint8_t address, uint8_t* buffer, uint32_t length);

void i2c_write_byte(uint8_t address, uint8_t byte);
void i2c_write_bytes(uint8_t address, uint8_t* buffer, uint32_t length);

/**
\}
\}
*/

#endif
