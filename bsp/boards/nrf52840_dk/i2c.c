/**
 * brief nrf52840-specific definition of the "i2c" bsp module.
 *
 * Authors: Tamas Harczos (tamas.harczos@imms.de)
 * Company: Institut fuer Mikroelektronik- und Mechatronik-Systeme gemeinnuetzige GmbH (IMMS GmbH)
 * Date:    August 2018
*/


#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "i2c.h"
#include "leds.h"
#include "board.h"
#include "board_info.h"

//=========================== defines =========================================


//=========================== variables =======================================

#if BOARD_PCA10056           // nrf52840-DK
#define I2C_SCL_PIN NRF_GPIO_PIN_MAP(1, 5)
#define I2C_SDA_PIN NRF_GPIO_PIN_MAP(1, 6)
#endif

#if BOARD_PCA10059           // nrf52840-DONGLE
#define I2C_SCL_PIN NRF_GPIO_PIN_MAP(1, 0)
#define I2C_SDA_PIN NRF_GPIO_PIN_MAP(0, 24)
#endif

//=========================== prototypes ======================================

//=========================== public ==========================================

void i2c_init(void) {
    
}

void i2c_read_registers(
    uint8_t bus_num,
    uint8_t slave_addr,
    uint8_t reg_addr,
    uint8_t numBytes,
    uint8_t* spaceToWrite
) {

}

void i2c_write_register(uint8_t bus_num, 
    uint8_t slave_addr, 
    uint8_t length, 
    uint8_t* data
) {
    
}

uint8_t i2c_slave_present(
    uint8_t bus_num,
    uint8_t slave_address
) {
    
}

void i2c_read_byte(
    uint8_t address, 
    uint8_t* byte
) {
    
}
uint32_t i2c_read_bytes(
    uint8_t address, 
    uint8_t* buffer, 
    uint32_t length
) {
    
}

void i2c_write_byte(uint8_t address, uint8_t byte) {
    
}

uint32_t i2c_write_bytes(
    uint8_t address, 
    uint8_t* buffer, 
    uint32_t length
) {

}

//=========================== private =========================================

