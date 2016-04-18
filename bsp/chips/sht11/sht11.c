/**
 * \file
 *         Device drivers for SHT11 temperature and humidity sensor in Telosb.
 * \author
 *         Pedro Henrique Gomes <pedrohenriquegomes@gmail.com>
 */

#include "stdint.h"
#include "stdbool.h"

#include "i2c.h"
#include "sht11.h"

//=========================== define ===========================================

//=========================== variables ========================================

//=========================== prototypes =======================================

//=========================== public ===========================================

void sht11_init(void) {
}

void sht11_reset(void) {
}

uint8_t sht11_is_present(void) {
    return 0;
}

uint16_t sht11_read_temperature(void) {
    return 0;
}

float sht11_convert_temperature(uint16_t temperature) {
    return 0;
}

uint16_t sht11_read_humidity(void) {
    return 0;
}

float sht11_convert_humidity(uint16_t humidity) {
    return 0;
}

//=========================== private ==========================================
