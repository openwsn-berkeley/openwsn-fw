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

#define I2C_PRESCALE        0x26       // I2C SCL speed (392kHz @ 16MHz)
#define I2C_BUS_FREE_TIME   100        // 100 ticks @8MHz ~ 12us
#define SDA_PIN             0x02       // msp430x261x UCB1SDA pin
#define SCL_PIN             0x04       // msp430x261x UCB1SCL pin

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void i2c_init(void);
void i2c_read_registers(uint8_t bus_num,uint8_t slave_addr,
                             uint8_t reg_addr,
                             uint8_t numBytes,
                             uint8_t* spaceToWrite);
void i2c_write_register(uint8_t bus_num, uint8_t slave_addr, uint8_t length, uint8_t* data);

uint8_t i2c_slave_present(uint8_t bus_num,uint8_t slave_address);

void i2c_read_byte(uint8_t address, uint8_t* byte);
uint32_t i2c_read_bytes(uint8_t address, uint8_t* buffer, uint32_t length);

void i2c_write_byte(uint8_t address, uint8_t byte);
uint32_t i2c_write_bytes(uint8_t address, uint8_t* buffer, uint32_t length);

// interrupt handlers
void isr_i2c_tx(uint8_t bus_num);
void isr_i2c_rx(uint8_t bus_num);

/**
\}
\}
*/

#endif
