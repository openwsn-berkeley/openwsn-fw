/**
 * \file
 *         Device drivers for SHT21 temperature and humidity sensor in OpenMote-CC2538.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#ifndef OPENWSN_SHT21_H
#define OPENWSN_SHT21_H

#include "opendefs.h"

void sht21_init(void);
void sht21_reset(void);
uint8_t sht21_is_present(void);
uint16_t sht21_read_temperature(void);
float sht21_convert_temperature(uint16_t temperature);
uint16_t sht21_read_humidity(void);
float sht21_convert_humidity(uint16_t humidity);

#endif /* OPENWSN_SHT21_H */


