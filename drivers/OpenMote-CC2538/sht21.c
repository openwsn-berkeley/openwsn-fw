/**
 * \file
 *         Device drivers for SHT21 temperature and humidity sensor in OpenMote-CC2538.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#include "stdint.h"
#include "stdbool.h"

#include "i2c.h"
#include "sht21.h"

//=========================== define ==========================================

#define SHT21_ADDRESS                   ( 0x40 )

#define SHT21_USER_REG_READ             ( 0xE7 )
#define SHT21_USER_REG_WRITE            ( 0xE6 )
#define SHT21_USER_REG_RESERVED_BITS    ( 0x38 )

#define SHT21_RESOLUTION_12b_14b        ( (0 << 7) | (0 << 0) )
#define SHT21_RESOLUTION_8b_12b         ( (0 << 7) | (1 << 0) )
#define SHT21_RESOLUTION_10b_13b        ( (1 << 7) | (0 << 0) )
#define SHT21_RESOLUTION_11b_11b        ( (1 << 7) | (1 << 0) )
#define SHT21_BATTERY_ABOVE_2V25        ( 0 << 6 )
#define SHT21_BATTERY_BELOW_2V25        ( 1 << 6 )
#define SHT21_ONCHIP_HEATER_ENABLE      ( 1 << 2 )
#define SHT21_ONCHIP_HEATER_DISABLE     ( 0 << 2 )
#define SHT21_OTP_RELOAD_ENABLE         ( 0 << 1 )
#define SHT21_OTP_RELOAD_DISABLE        ( 1 << 1 )

#define SHT21_TEMPERATURE_HM_CMD        ( 0xE3 )
#define SHT21_HUMIDITY_HM_CMD           ( 0xE5 )
#define SHT21_TEMPERATURE_NHM_CMD       ( 0xF3 )
#define SHT21_HUMIDITY_NHM_CMD          ( 0xF5 )
#define SHT21_RESET_CMD                 ( 0xFE )

#define SHT21_STATUS_MASK               ( 0xFC )

#define SHT21_DEFAULT_CONFIG            ( SHT21_RESOLUTION_12b_14b | \
                                          SHT21_ONCHIP_HEATER_DISABLE | \
                                          SHT21_BATTERY_ABOVE_2V25 | \
                                          SHT21_OTP_RELOAD_DISABLE )

#define SHT21_USER_CONFIG               ( SHT21_RESOLUTION_8b_12b | \
                                          SHT21_ONCHIP_HEATER_DISABLE | \
                                          SHT21_BATTERY_ABOVE_2V25 | \
                                          SHT21_OTP_RELOAD_DISABLE )

//=========================== variables =======================================

//=========================== prototypes ======================================

static void sht21_pre_init(void);

//=========================== public ==========================================

void sht21_init(void) {
    uint8_t config[2];

    // Pre-init the STH21 if required
    sht21_pre_init();

    // Setup the configuration vector, the first position holds address
    // and the second position holds the actual configuration
    config[0] = SHT21_USER_REG_WRITE;
    config[1] = 0;

    // Read the current configuration according to the datasheet (pag. 9, fig. 18)
    i2c_write_byte(SHT21_ADDRESS, SHT21_USER_REG_READ);
    i2c_read_byte(SHT21_ADDRESS, &config[1]);

    // Clean all the configuration bits except those reserved
    config[1] &= SHT21_USER_REG_RESERVED_BITS;

    // Set the configuration bits without changing those reserved
    config[1] |= SHT21_USER_CONFIG;

    i2c_write_bytes(SHT21_ADDRESS, config, sizeof(config));
}

void sht21_reset(void) {
    // Send a soft-reset command according to the datasheet (pag. 9, fig. 17)
    i2c_write_byte(SHT21_ADDRESS, SHT21_RESET_CMD);
}

uint8_t sht21_is_present(void) {
    uint8_t is_present;

    // Pre-init the STH21 if required
    sht21_pre_init();

    // Read the current configuration according to the datasheet (pag. 9, fig. 18)
    i2c_write_byte(SHT21_ADDRESS, SHT21_USER_REG_READ);
    i2c_read_byte(SHT21_ADDRESS, &is_present);

    // Clear the reserved bits according to the datasheet (pag. 9, tab. 8)
    is_present &= ~SHT21_USER_REG_RESERVED_BITS;

    return (is_present == SHT21_DEFAULT_CONFIG || is_present == SHT21_USER_CONFIG);
}

uint16_t sht21_read_temperature(void) {
    uint8_t sht21_temperature[2];
    uint16_t temperature;

    // Read the current temperature according to the datasheet (pag. 8, fig. 15)
    i2c_write_byte(SHT21_ADDRESS, SHT21_TEMPERATURE_HM_CMD);
    i2c_read_bytes(SHT21_ADDRESS, sht21_temperature, sizeof(sht21_temperature));
    
    temperature = (sht21_temperature[0] << 8) | (sht21_temperature[1] & SHT21_STATUS_MASK);
    
    return temperature;
}

float sht21_convert_temperature(uint16_t temperature) {
    float result;
    
    result  = -46.85;
    result += 175.72 * temperature / 65536;
    
    return result;
}

uint16_t sht21_read_humidity(void) {
    uint8_t sht21_humidity[2];
    uint16_t humidity;

    // Read the current humidity according to the datasheet (pag. 8, fig. 15)
    i2c_write_byte(SHT21_ADDRESS, SHT21_HUMIDITY_HM_CMD);
    i2c_read_bytes(SHT21_ADDRESS, sht21_humidity, sizeof(sht21_humidity));

    humidity = (sht21_humidity[0] << 8) | (sht21_humidity[1] & SHT21_STATUS_MASK);

    return humidity;
}

float sht21_convert_humidity(uint16_t humidity) {
    float result;
    
    result  = -6.0;
    result += 125.0 * humidity / 65536;
    
    return result;
}

//=========================== private =========================================

static void sht21_pre_init(void) {
    static bool is_initialized = false;
    volatile uint32_t i;

    if (is_initialized == false) {
        // Delay needed for the SHT21 to startup
        for (i = 0x1FFFF; i != 0; i--);
        is_initialized = true;
    }
}
