/**
 * \file
 *         Device drivers for SI70X temperature and humidity sensor in OpenMote-CC2538.
 * \author
 *         Xavi Vilajosana, xvilajosana@eecs.berkeley.edu
 */

#include "stdint.h"
#include "stdbool.h"

#include "i2c.h"
#include "si70x.h"

//=========================== define ==========================================

#define SI70X_ADDRESS                   ( 0x40 )

#define SI70X_USER_REG_READ             ( 0xE7 )
#define SI70X_USER_REG_WRITE            ( 0xE6 )
#define SI70X_USER_REG_RESERVED_BITS    ( 0x38 )

#define SI70X_RESOLUTION_12b_14b        ( (0 << 7) | (0 << 0) )
#define SI70X_RESOLUTION_8b_12b         ( (0 << 7) | (1 << 0) )
#define SI70X_RESOLUTION_10b_13b        ( (1 << 7) | (0 << 0) )
#define SI70X_RESOLUTION_11b_11b        ( (1 << 7) | (1 << 0) )
#define SI70X_BATTERY_ABOVE_2V25        ( 0 << 6 )
#define SI70X_BATTERY_BELOW_2V25        ( 1 << 6 )
#define SI70X_ONCHIP_HEATER_ENABLE      ( 1 << 2 )
#define SI70X_ONCHIP_HEATER_DISABLE     ( 0 << 2 )
#define SI70X_OTP_RELOAD_ENABLE         ( 0 << 1 )
#define SI70X_OTP_RELOAD_DISABLE        ( 1 << 1 )

#define SI70X_TEMPERATURE_HM_CMD        ( 0xE3 )
#define SI70X_HUMIDITY_HM_CMD           ( 0xE5 )
#define SI70X_TEMPERATURE_NHM_CMD       ( 0xF3 )
#define SI70X_HUMIDITY_NHM_CMD          ( 0xF5 )
#define SI70X_RESET_CMD                 ( 0xFE )

#define SI70X_STATUS_MASK               ( 0xFC )

#define SI70X_DEFAULT_CONFIG            ( SI70X_RESOLUTION_12b_14b | \
                                          SI70X_ONCHIP_HEATER_DISABLE | \
                                          SI70X_BATTERY_ABOVE_2V25 | \
                                          SI70X_OTP_RELOAD_DISABLE )

#define SI70X_USER_CONFIG               ( SI70X_RESOLUTION_8b_12b | \
                                          SI70X_ONCHIP_HEATER_DISABLE | \
                                          SI70X_BATTERY_ABOVE_2V25 | \
                                          SI70X_OTP_RELOAD_DISABLE )

//=========================== variables =======================================

//=========================== prototypes ======================================

static void si70x_pre_init(void);

//=========================== public ==========================================

void si70x_init(void) {
    uint8_t config[2];

    // Pre-init the STH21 if required
    si70x_pre_init();

    // Setup the configuration vector, the first position holds address
    // and the second position holds the actual configuration
    config[0] = SI70X_USER_REG_WRITE;
    config[1] = 0;

    // Read the current configuration according to the datasheet (pag. 9, fig. 18)
    i2c_write_byte(SI70X_ADDRESS, SI70X_USER_REG_READ);
    i2c_read_byte(SI70X_ADDRESS, &config[1]);

    // Clean all the configuration bits except those reserved
    config[1] &= SI70X_USER_REG_RESERVED_BITS;

    // Set the configuration bits without changing those reserved
    config[1] |= SI70X_USER_CONFIG;

    i2c_write_bytes(SI70X_ADDRESS, config, sizeof(config));
}

void si70x_reset(void) {
    // Send a soft-reset command according to the datasheet (pag. 9, fig. 17)
    i2c_write_byte(SI70X_ADDRESS, SI70X_RESET_CMD);
}

uint8_t si70x_is_present(void) {
    uint8_t is_present;

    // Pre-init the STH21 if required
    si70x_pre_init();

    // Read the current configuration according to the datasheet (pag. 9, fig. 18)
    i2c_write_byte(SI70X_ADDRESS, SI70X_USER_REG_READ);
    i2c_read_byte(SI70X_ADDRESS, &is_present);

    // Clear the reserved bits according to the datasheet (pag. 9, tab. 8)
    is_present &= ~SI70X_USER_REG_RESERVED_BITS;

    return (is_present == SI70X_DEFAULT_CONFIG || is_present == SI70X_USER_CONFIG);
}

uint16_t si70x_read_temperature(void) {
    uint8_t si70x_temperature[2];
    uint16_t temperature;

    // Read the current temperature according to the datasheet (pag. 8, fig. 15)
    i2c_write_byte(SI70X_ADDRESS, SI70X_TEMPERATURE_HM_CMD);
    i2c_read_bytes(SI70X_ADDRESS, si70x_temperature, sizeof(si70x_temperature));
    
    temperature = (si70x_temperature[0] << 8) | (si70x_temperature[1] & SI70X_STATUS_MASK);
    
    return temperature;
}

float si70x_convert_temperature(uint16_t temperature) {
    float result;
    
    result  = -46.85;
    result += 175.72 * temperature / 65536;
    
    return result;
}

uint16_t si70x_read_humidity(void) {
    uint8_t si70x_humidity[2];
    uint16_t humidity;

    // Read the current humidity according to the datasheet (pag. 8, fig. 15)
    i2c_write_byte(SI70X_ADDRESS, SI70X_HUMIDITY_HM_CMD);
    i2c_read_bytes(SI70X_ADDRESS, si70x_humidity, sizeof(si70x_humidity));

    humidity = (si70x_humidity[0] << 8) | (si70x_humidity[1] & SI70X_STATUS_MASK);

    return humidity;
}

float si70x_convert_humidity(uint16_t humidity) {
    float result;
    
    result  = -6.0;
    result += 125.0 * humidity / 65536;
    
    return result;
}

//=========================== private =========================================

static void si70x_pre_init(void) {
    static bool is_initialized = false;
    volatile uint32_t i;

    if (is_initialized == false) {
        // Delay needed for the SI70X to startup
        for (i = 0x1FFFF; i != 0; i--);
        is_initialized = true;
    }
}
