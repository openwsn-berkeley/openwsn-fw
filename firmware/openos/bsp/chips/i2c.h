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

void i2c_init();
void i2c_read_registers(uint8_t bus_num,uint8_t slave_addr,
                             uint8_t reg_addr,
                             uint8_t numBytes,
                             uint8_t* spaceToWrite);
void i2c_write_register(uint8_t bus_num,uint8_t slave_addr,
                             uint8_t reg_addr,
                             uint8_t reg_setting);
unsigned char i2c_slave_present(int bus_num,unsigned char slave_address);

// interrupt handlers
void isr_i2c_tx(int bus_num);
void isr_i2c_rx(int bus_num);

/**
\}
\}
*/

#endif
