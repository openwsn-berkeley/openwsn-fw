/**
 * \file
 *         Device drivers for MAX44009 light sensor in OpenMote-CC2538.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#include "i2c.h"
#include "max44009.h"

//=========================== define ==========================================

/* ADDRESS AND NOT_FOUND VALUE */
#define MAX44009_ADDRESS                    ( 0x4A )
#define MAX44009_NOT_FOUND                  ( 0x00 )

/* REGISTER ADDRESSES */
#define MAX44009_INT_STATUS_ADDR            ( 0x00 )    // R
#define MAX44009_INT_ENABLE_ADDR            ( 0x01 )    // R/W
#define MAX44009_CONFIG_ADDR                ( 0x02 )    // R/W
#define MAX44009_LUX_HIGH_ADDR              ( 0x03 )    // R
#define MAX44009_LUX_LOW_ADDR               ( 0x04 )    // R
#define MAX44009_THR_HIGH_ADDR              ( 0x05 )    // R/W
#define MAX44009_THR_LOW_ADDR               ( 0x06 )    // R/W
#define MAX44009_THR_TIMER_ADDR             ( 0x07 )    // R/W

/* INTERRUPT VALUES */
#define MAX44009_INT_STATUS_OFF             ( 0x00 )
#define MAX44009_INT_STATUS_ON              ( 0x01 )
#define MAX44009_INT_DISABLED               ( 0x00 )
#define MAX44009_INT_ENABLED                ( 0x01 )

/* CONFIGURATION VALUES */
#define MAX44009_CONFIG_DEFAULT             ( 0 << 7 )
#define MAX44009_CONFIG_CONTINUOUS          ( 1 << 7 )
#define MAX44009_CONFIG_AUTO                ( 0 << 6 )
#define MAX44009_CONFIG_MANUAL              ( 1 << 6 )
#define MAX44009_CONFIG_CDR_NORMAL          ( 0 << 3 )
#define MAX44009_CONFIG_CDR_DIVIDED         ( 1 << 3 )
#define MAX44009_CONFIG_INTEGRATION_800ms   ( 0 << 0 )
#define MAX44009_CONFIG_INTEGRATION_400ms   ( 1 << 0 )
#define MAX44009_CONFIG_INTEGRATION_200ms   ( 2 << 0 )
#define MAX44009_CONFIG_INTEGRATION_100ms   ( 3 << 0 )
#define MAX44009_CONFIG_INTEGRATION_50ms    ( 4 << 0 )
#define MAX44009_CONFIG_INTEGRATION_25ms    ( 5 << 0 )
#define MAX44009_CONFIG_INTEGRATION_12ms    ( 6 << 0 )
#define MAX44009_CONFIG_INTEGRATION_6ms     ( 7 << 0 )

/* DEFAULT CONFIGURATION */
#define MAX44009_DEFAULT_CONFIGURATION      ( MAX44009_CONFIG_DEFAULT | \
                                              MAX44009_CONFIG_AUTO | \
                                              MAX44009_CONFIG_CDR_NORMAL | \
                                              MAX44009_CONFIG_INTEGRATION_100ms )

/* USER CONFIGURATION */
#define MAX44009_USER_CONFIGURATION         ( MAX44009_CONFIG_DEFAULT | \
                                              MAX44009_CONFIG_MANUAL | \
                                              MAX44009_CONFIG_CDR_NORMAL | \
                                              MAX44009_CONFIG_INTEGRATION_100ms )

//=========================== variables =======================================


//=========================== prototypes ======================================


//=========================== public ==========================================

void max44009_init(void) {
    uint8_t max44009_address[5] = {MAX44009_INT_ENABLE_ADDR, MAX44009_CONFIG_ADDR, \
                                   MAX44009_THR_HIGH_ADDR, MAX44009_THR_LOW_ADDR, \
                                   MAX44009_THR_TIMER_ADDR};
    uint8_t max44009_value[5]   = {MAX44009_INT_DISABLED, MAX44009_USER_CONFIGURATION, \
                                   0xFF, 0x00, 0xFF};
    uint8_t max44009_data[2];
    uint8_t i;

    for (i = 0; i < sizeof(max44009_address); i++) {
        max44009_data[0] = max44009_address[i];
        max44009_data[1] = max44009_value[i];
        i2c_write_bytes(MAX44009_ADDRESS, max44009_data, sizeof(max44009_data));
    }
}

void max44009_reset(void) {
    uint8_t max44009_address[5] = {MAX44009_INT_ENABLE_ADDR, MAX44009_CONFIG_ADDR, \
                                   MAX44009_THR_HIGH_ADDR, MAX44009_THR_LOW_ADDR, \
                                   MAX44009_THR_TIMER_ADDR};
    uint8_t max44009_value[5]   = {MAX44009_INT_DISABLED, MAX44009_DEFAULT_CONFIGURATION, \
                                   0xFF, 0x00, 0xFF};
    uint8_t max44009_data[2];
    uint8_t i;
    
    for (i = 0; i < sizeof(max44009_address); i++) {
        max44009_data[0] = max44009_address[i];
        max44009_data[1] = max44009_value[i];
        i2c_write_bytes(MAX44009_ADDRESS, max44009_data, sizeof(max44009_data));
    }
}

uint8_t max44009_is_present(void) {
    uint8_t is_present;

    i2c_write_byte(MAX44009_ADDRESS, MAX44009_CONFIG_ADDR);
    i2c_read_byte(MAX44009_ADDRESS, &is_present);

    return (is_present == MAX44009_DEFAULT_CONFIGURATION ||
            is_present == MAX44009_USER_CONFIGURATION);
}

uint16_t max44009_read_light(void) {
    uint8_t exponent, mantissa;
    uint8_t max44009_data[2];
    uint16_t result;

    i2c_write_byte(MAX44009_ADDRESS, MAX44009_LUX_HIGH_ADDR);
    i2c_read_byte(MAX44009_ADDRESS, &max44009_data[0]);
    i2c_write_byte(MAX44009_ADDRESS, MAX44009_LUX_LOW_ADDR);
    i2c_read_byte(MAX44009_ADDRESS, &max44009_data[1]);

    exponent = (( max44009_data[0] >> 4 )  & 0x0F);
    mantissa = (( max44009_data[0] & 0x0F ) << 4) | ((max44009_data[1] & 0x0F));

    result = ( (uint16_t) exponent << 8 ) | ( (uint16_t) mantissa << 0);
    
    return result;
}

float max44009_convert_light(uint16_t lux) {
    uint8_t exponent, mantissa;
    float result = 0.045;

    exponent = (lux >> 8) & 0xFF;    
    exponent = (exponent == 0x0F ? exponent & 0x0E : exponent);

    mantissa = (lux >> 0) & 0xFF;
    
    result *= 2^exponent * mantissa;
    
    return result;
}

//=========================== private =========================================
